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
char *days[7]={"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};

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
  char perm[10], buf[512], str[2048], *rstr;
  int i=0;
  struct stat d;
  struct tm *tm;
  struct dirent *dp;
  DIR *dir;

  dir=opendir(c);
  memset(str, '\0', 1024);
  while((dp=readdir(dir))!=NULL){
    for(i=0;i<10;i++){perm[i]='-';}
    stat(dp->d_name, &d);
    tm=gmtime(&d.st_mtime);
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
    sprintf(buf, " %s %s %2d %2d:%2d:%2d %4d %s\n", days[tm->tm_wday], months[tm->tm_mon], tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, tm->tm_year+1900, dp->d_name);
    strcat(str, buf);
  }
  rstr=str;
  return rstr;
}

main(int argc, char *argv[])
{
  const char *hdir;
  char *hostname, *str;
  char line[MAX], cwd[128], ncwd[128];
  int nint=0, i =0, j=0;
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


      //pwd
      if(!strncmp(line, "pwd", 3)){
        getcwd(cwd, 128);
        str = cwd;
      }


      //ls
      if(!strncmp(line, "ls", 2)){
        getcwd(cwd, 128);
        if(strlen(line)<3){//only ls
          str = printdir(cwd);//ls cwd
        }
        else{
          if(line[3]!='/'){
            strcat(cwd, "/");
            for(i=3;i<=strlen(line);i++){
              strncat(cwd, &line[i], 1);
            }
            str = printdir(cwd);
          }
          else{
            if(line[4]=='\n'){
              printf("found root printy time");
            }
            printf("%d\n", strlen(line));
            for(i=3;i<=strlen(line);i++){
              putchar(line[i]);
              strncat(ncwd, &line[i], 1);
            }
            strcat(ncwd, "\n");
            str = printdir(ncwd);
          }
        }
      }


      //cd
      if(!strncmp(line, "cd", 2)){
        if(strlen(line)<3){//only cd
          chdir(hdir);//go to HOME
          getcwd(cwd, 128);
          str = cwd;
        }
        else{
          if(line[3]=='/'){//new path
            sscanf(line ,"cd %s", ncwd);
            strncpy(str, ncwd, strlen(ncwd));
            chdir(str);
          }
          else{//go from cwd
            getcwd(cwd, 128);
            strcat(cwd, "/");
            sscanf(line ,"cd %s", ncwd);
            strcat(cwd, ncwd);
            strncpy(str, cwd, strlen(cwd));
            chdir(str);
          }
        }
      }


      //cat
      // else if(!strncmp(line, "cat", 3)){
      //   if(strlen(entry[1].value)){
      //     if(entry[1].value[0]!='/'){
      //       getcwd(cwd, 128);
      //       strcat(cwd, "/");
      //       strcat(cwd, entry[1].value);
      //       fp=fopen(cwd, "r");
      //     }
      //     else{
      //       fp=fopen(line, "r");
      //     }
      //     if(fp==NULL){
      //       printf("No such file<p>");
      //     }
      //     else{
      //       while((c=fgetc(fp)) != EOF){
      //         putchar(c);
      //       }
      //     }
      //     printf("<p>");
      //     fclose(fp);
      //   }
      // }


      //mkdir
      // else if(!strncmp(line, "mkdir", 5)){
      //   if(strlen(entry[1].value)){
      //     t=mkdir(entry[1].value, 0755);
      //     if(t<0){
      //       printf("mkdir failed<p>");
      //     }
      //   }
      // }


      //rmdir
      // else if(!strncmp(line, "rmdir", 5)){
      //   if(strlen(entry[1].value)){
      //     t=rmdir(entry[1].value);
      //     if(t<0){
      //       printf("rmdir failed<p>");
      //     }
      //   }
      // }


      //rm
      // else if(!strncmp(line, "rm", 2)){
      //   if(strlen(entry[1].value)){
      //     t=remove(entry[1].value);
      //     if(t<0){
      //       printf("rm failed<p>");
      //     }
      //   }
      // }


      //cp
      // else if(!strncmp(line, "cp", 2)){
      //   if(strlen(entry[1].value)){
      //     if(strlen(entry[2].value)){
      //       if(entry[1].value[0]!='/'){
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
      //     }
      //   }
      // }





      // send the echo line to client
      nint=htonl(((strlen(str)*sizeof(char))+1));
      //first send length of message

      n = write(client_sock, &nint, sizeof(nint));
      printf("%d\n", ntohl(nint));
      snprintf(line, ntohl(nint),"%s", str);
      n = write(client_sock, line, ntohl(nint));
      printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
      memset(line, '\0', ntohl(nint));
      memset(str, '\0', strlen(str));
      printf("server: ready for next request\n");
    }
  }
}
