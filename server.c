// This is the echo SERVER server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>

#include <sys/socket.h>
#include <netdb.h>

#define  MAX 256

// Define variables:
struct sockaddr_in  server_addr, client_addr, name_addr;
struct hostent *hp;

int  mysock, client_sock;              // socket descriptors
int  serverPort;                     // server port number
int  r, length, n;                   // help variables

char *months[12]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
char *days[7]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

// Server initialization code:

int server_init(char *name)
{
  printf("==================== server init ======================\n");
  // get DOT name and IP address of this host

  printf("1 : get and show server host info\n");
  hp = gethostbyname(name);
  if (hp == 0){
    printf("unknown host\n");
    exit(1);
  }
  printf("    hostname=%s  IP=%s\n",
  hp->h_name,  inet_ntoa(*(long *)hp->h_addr));

  //  create a TCP socket by socket() syscall
  printf("2 : create a socket\n");
  mysock = socket(AF_INET, SOCK_STREAM, 0);
  if (mysock < 0){
    printf("socket call failed\n");
    exit(2);
  }

  printf("3 : fill server_addr with host IP and PORT# info\n");
  // initialize the server_addr structure
  server_addr.sin_family = AF_INET;                  // for TCP/IP
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   // THIS HOST IP address
  server_addr.sin_port = 0;   // let kernel assign port

  printf("4 : bind socket to host info\n");
  // bind syscall: bind the socket to server_addr info
  r = bind(mysock,(struct sockaddr *)&server_addr, sizeof(server_addr));
  if (r < 0){
    printf("bind failed\n");
    exit(3);
  }

  printf("5 : find out Kernel assigned PORT# and show it\n");
  // find out socket port number (assigned by kernel)
  length = sizeof(name_addr);
  r = getsockname(mysock, (struct sockaddr *)&name_addr, &length);
  if (r < 0){
    printf("get socketname error\n");
    exit(4);
  }

  // show port number
  serverPort = ntohs(name_addr.sin_port);   // convert to host ushort
  printf("    Port=%d\n", serverPort);

  // listen at port with a max. queue of 5 (waiting clients)
  printf("5 : server is listening ....\n");
  listen(mysock, 5);
  printf("===================== init done =======================\n");
}

char *printdir(char *c){
  int i=0;
  struct stat d;
  struct tm tm;
  struct dirent *dp;
  DIR *dir;
  char buf[2048], str[2048], *rstr, tim[80];

  dir=opendir(c);
  memset(str, '\0', 2048);
  memset(buf, '\0', 2048);
  while((dp=readdir(dir))!=NULL){
    stat(dp->d_name, &d);
    localtime_r(&d.st_mtime, &tm);
    strftime(tim, sizeof(tim), "%c", &tm);

    sprintf(buf, (S_ISDIR(d.st_mode)) ? "d" : "-");
    strcat(str, buf);
    sprintf(buf, (d.st_mode & S_IRUSR) ? "r" : "-");
    strcat(str, buf);
    sprintf(buf, (d.st_mode & S_IWUSR) ? "w" : "-");
    strcat(str, buf);
    sprintf(buf, (d.st_mode & S_IXUSR) ? "x" : "-");
    strcat(str, buf);
    sprintf(buf, (d.st_mode & S_IRGRP) ? "r" : "-");
    strcat(str, buf);
    sprintf(buf, (d.st_mode & S_IWGRP) ? "w" : "-");
    strcat(str, buf);
    sprintf(buf, (d.st_mode & S_IXGRP) ? "x" : "-");
    strcat(str, buf);
    sprintf(buf, (d.st_mode & S_IROTH) ? "r" : "-");
    strcat(str, buf);
    sprintf(buf, (d.st_mode & S_IWOTH) ? "w" : "-");
    strcat(str, buf);
    sprintf(buf, (d.st_mode & S_IXOTH) ? "x" : "-");
    strcat(str, buf);

    sprintf(buf, "%4d %4d %4d %6d %s %s\n", d.st_nlink, d.st_uid, d.st_gid, d.st_size, tim, dp->d_name);
    strcat(str, buf);

  }
  rstr=str;
  return rstr;
}

main(int argc, char *argv[])
{
  const char *hdir;
  FILE *fp;
  struct stat d;
  char *hostname, *str, *fstr;
  char line[MAX], cwd[128], ncwd[128], buf[512], c;
  int nint=0, i =0, j=0, t=0;
  if (argc < 2)
  hostname = "localhost";
  else
  hostname = argv[1];

  server_init(hostname);
  hdir=getpwuid(getuid())->pw_dir;
  printf("HOME %s\n", hdir);
  // Try to accept a client request
  while(1){
    printf("server: accepting new connection ....\n");

    // Try to accept a client connection as descriptor newsock
    length = sizeof(client_addr);
    client_sock = accept(mysock, (struct sockaddr *)&client_addr, &length);
    if (client_sock < 0){
      printf("server: accept error\n");
      exit(1);
    }
    printf("server: accepted a client connection from\n");
    printf("-----------------------------------------------\n");
    printf("        IP=%s  port=%d\n", inet_ntoa(client_addr.sin_addr.s_addr),
    ntohs(client_addr.sin_port));
    printf("-----------------------------------------------\n");

    // Processing loop: newsock <----> client
    while(1){
      n = read(client_sock, line, MAX);
      if (n==0){
        printf("server: client died, server loops\n");
        close(client_sock);
        break;
      }

      // show the line string
      printf("server: read  n=%d bytes; line=[%s]\n", n, line);
      if(!strncmp(line, "get", 3)){
        if(strlen(line)>4){
          if(line[4]!='/'){
            getcwd(cwd, 128);
            strcat(cwd, "/");
            sscanf(line, "get %s", ncwd);
            strcat(cwd, ncwd);
          }
          else{
            sscanf(line, "get %s", cwd);
          }
      }
      stat(cwd, &d);
      nint=htonl((d.st_size*sizeof(char)));
      fstr = malloc(ntohl(nint)*sizeof(char));

      //first send length of message
      sprintf(str, "%d %s", nint, cwd);
      //print"f("%d\n", ntohl(nint));
      n = write(client_sock, str, MAX);
      fp = fopen(cwd, "r");

      n = write(client_sock, fstr, ntohl(nint));
      free(fstr);
    }
else{

      //pwd
      if(!strncmp(line, "pwd", 3)){
        getcwd(cwd, 128);
        str = cwd;
      }


      //ls

      /*bug in ls
      when ls any but current directory, permissions and date incorrect*/
      if(!strncmp(line, "ls", 2)){

        getcwd(cwd, 128);
        if(strlen(line)<3){//only ls
          str = printdir(cwd);//ls cwd
        }
        else{
          if(line[3]!='/'){
            strcat(cwd, "/");
            sscanf(line, "ls %s", ncwd);
            strcat(cwd, ncwd);
            //printf("cwd %s", cwd);
            str = printdir(cwd);
            //printf("\n%s\n",cwd);
          }
          else if(line[4]=='\n'){
            str = printdir(line[3]);
          }
          else{
            sscanf(line, "ls %s", ncwd);
            strcpy(cwd, ncwd);
            //printf("cwd %s", cwd);
            str = printdir(cwd);
            //printf("\n%s\n",cwd);

          }
        }
      }


      //cd
      if(!strncmp(line, "cd", 2)){
        if(strlen(line)<3){//only cd
          chdir(hdir);//go to HOME
          strcpy(str, hdir);
        }
        else{
          if(line[3]!='/'){//go from cwd
            getcwd(cwd, 128);
            strcat(cwd, "/");
            sscanf(line ,"cd %s", ncwd);
            strcat(cwd, ncwd);
            strcpy(str, cwd);
            chdir(str);
          }
          else{//new path
            sscanf(line ,"cd %s", ncwd);
            strcpy(str, ncwd);
            chdir(str);
          }
        }
      }




      //mkdir
      else if(!strncmp(line, "mkdir", 5)){
        if(strlen(line)>6){
          if(line[6]!='/'){
            getcwd(cwd, 128);
            strcat(cwd, "/");
            sscanf(line, "mkdir %s", ncwd);
            strcat(cwd, ncwd);
            strcpy(str, cwd);
          }

          else{
            sscanf(line, "mkdir %s", cwd);
            strcpy(str, cwd);
          }
          t=mkdir(cwd, 0755);
          if(t<0){
            printf("mkdir failed\n");
          }
        }
      }


      //rmdir
      else if(!strncmp(line, "rmdir", 5)){
        if(strlen(line)>6){
          if(line[6]!='/'){
            getcwd(cwd, 128);
            strcat(cwd, "/");
            sscanf(line, "rmdir %s", ncwd);
            strcat(cwd, ncwd);
            strcpy(str, cwd);

          }
          else if(line[7]=='\n'){
            strcpy(cwd, "/");
            strcpy(str, cwd);
          }
          else{
            sscanf(line, "rmdir %s", cwd);
            strcpy(str, cwd);
          }
          t=rmdir(cwd, 0755);
          if(t<0){
            printf("mkdir failed\n");
          }
        }
      }



      //rm
      else if(!strncmp(line, "rm", 2)){
        if(strlen(line)>3){
          if(line[3]!='/'){
            getcwd(cwd, 128);
            strcat(cwd, "/");
            sscanf(line, "rmdir %s", ncwd);
            strcat(cwd, ncwd);
            strcpy(str, cwd);

          }
          else if(line[4]=='\n'){
            strcpy(cwd, "/");
            strcpy(str, cwd);
          }
          else{
            sscanf(line, "rmdir %s", cwd);
            strcpy(str, cwd);
          }
          t=remove(cwd);
          if(t<0){
            printf("rm failed\n");
          }
        }
        else{
          strcpy(str, "incorrect command, use\nrm PATH\n");
        }
      }


      //get
      //cp
      // else if(!strncmp(line, "cp", 2)){
      //   if(strlen(line)>3){
      //
      //       if(line[3]!='/'){
      //         getcwd(cwd, 128);
      //         strcat(cwd, "/");
      //         strcat(cwd, entry[1].value);
      //         fp=fopen(cwd, "r");
      //       }
      //       if(entry[2].value[0]!='/'){
      //         getcwd(cwd, 128);
      //         printf("cwd%s<p>", cwd);
      //         strcat(cwd, "/");
      //         strcat(cwd, entry[2].value);
      //         printf("%s<p>", cwd);
      //         fpc=fopen(cwd, "ab+");
      //       }
      //       if(fp==NULL){
      //         printf("filename1 open error");
      //       }
      //       if(fpc==NULL){
      //         printf("filename2 open error");
      //       }
      //       while((c=fgetc(fp))!=EOF){
      //         fputc(c, fpc);
      //       }
      //       printf("<p>");
      //
      //   }
      // }


      // send the echo line to client
      nint=htonl(((strlen(str)*sizeof(char))+1));
      //first send length of message
      n = write(client_sock, &nint, MAX);
      //print"f("%d\n", ntohl(nint));
      n = write(client_sock, str, ntohl(nint));
      printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, str);

      memset(cwd, '\0', 128);
      memset(ncwd, '\0', 128);
      memset(line, '\0', 256);
      memset(str, '\0', strlen(str));
      printf("server: ready for next request\n");
    }
    }
  }
}
