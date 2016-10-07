// The echo client client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAX 256

// Define variables
struct hostent *hp;
struct sockaddr_in  server_addr;

int server_sock, r;
int SERVER_IP, SERVER_PORT;


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


// clinet initialization code

int client_init(char *argv[])
{
  printf("======= clinet init ==========\n");

  printf("1 : get server info\n");
  hp = gethostbyname(argv[1]);
  if (hp==0){
     printf("unknown host %s\n", argv[1]);
     exit(1);
  }

  SERVER_IP   = *(long *)hp->h_addr;
  SERVER_PORT = atoi(argv[2]);

  printf("2 : create a TCP socket\n");
  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock<0){
     printf("socket call failed\n");
     exit(2);
  }

  printf("3 : fill server_addr with server's IP and PORT#\n");
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = SERVER_IP;
  server_addr.sin_port = htons(SERVER_PORT);

  // Connect to server
  printf("4 : connecting to server ....\n");
  r = connect(server_sock,(struct sockaddr *)&server_addr, sizeof(server_addr));
  if (r < 0){
     printf("connect failed\n");
     exit(1);
  }

  printf("5 : connected OK to \007\n");
  printf("---------------------------------------------------------\n");
  printf("hostname=%s  IP=%s  PORT=%d\n",
          hp->h_name, inet_ntoa(SERVER_IP), SERVER_PORT);
  printf("---------------------------------------------------------\n");

  printf("========= init done ==========\n");
}

main(int argc, char *argv[ ])
{
  int n, nint=0, i=0, t=0;
  FILE *fp;
  char line[MAX], ans[MAX], cwd[128], ncwd[128], *str, *hdir, *fstr;

  if (argc < 3){
     printf("Usage : client ServerName SeverPort\n");
     exit(1);
  }

  client_init(argv);
  // sock <---> server
  hdir=getpwuid(getuid())->pw_dir;
  printf("********  processing loop  *********\n");
  while (1){
    printf("input a line : ");
    bzero(line, MAX);                // zero out line[ ]
    fgets(line, MAX, stdin);         // get a line (end with \n) from stdin

    line[strlen(line)-1] = 0;        // kill \n at end
    if (line[0]==0)                  // exit if NULL line
       exit(0);
    //if local command
      if(!strncmp(line, "lpwd", 4)){
        getcwd(cwd, 128);
        printf("%s\n", cwd);
      }


      //ls

      /*bug in ls
      when ls any but current directory, permissions and date incorrect*/
      if(!strncmp(line, "lls", 3)){

        getcwd(cwd, 128);
        if(strlen(line)<4){//only ls
          printf("%s\n", printdir(cwd));//ls cwd
        }
        else{
          if(line[4]!='/'){
            strcat(cwd, "/");
            sscanf(line, "lls %s", ncwd);
            strcat(cwd, ncwd);
            //printf("cwd %s", cwd);
            printf("%s\n", printdir(cwd));
            //printf("\n%s\n",cwd);
          }
          else if(line[5]=='\n'){
            printf("%s\n", printdir(line[4]));
          }
          else{
            sscanf(line, "lls %s", ncwd);
            strcpy(cwd, ncwd);
            //printf("cwd %s", cwd);
            printf("%s\n", printdir(cwd));
            //printf("\n%s\n",cwd);

          }
        }
      }


      //cd
      if(!strncmp(line, "lcd", 3)){
        if(strlen(line)<4){//only cd
          chdir(hdir);//go to HOME
        }
        else{
          if(line[4]!='/'){//go from cwd
            getcwd(cwd, 128);
            strcat(cwd, "/");
            sscanf(line ,"lcd %s", ncwd);
            strcat(cwd, ncwd);
            chdir(cwd);
          }
          else{//new path
            sscanf(line ,"lcd %s", ncwd);
            chdir(ncwd);
          }
        }
      }




      //mkdir
      else if(!strncmp(line, "lmkdir", 6)){
        if(strlen(line)>7){
          if(line[7]!='/'){
            getcwd(cwd, 128);
            strcat(cwd, "/");
            sscanf(line, "lmkdir %s", ncwd);
            strcat(cwd, ncwd);
          }

          else{
            sscanf(line, "lmkdir %s", cwd);
          }
          t=mkdir(cwd, 0755);
          if(t<0){
            printf("lmkdir failed\n");
          }
          else{
            printf("new dir at %s\n", cwd);
          }
        }
      }


      //rmdir
      else if(!strncmp(line, "lrmdir", 6)){
        if(strlen(line)>7){
          if(line[7]!='/'){
            getcwd(cwd, 128);
            strcat(cwd, "/");
            sscanf(line, "lrmdir %s", ncwd);
            strcat(cwd, ncwd);

          }
          else{
            sscanf(line, "lrmdir %s", cwd);
          }
          t=rmdir(cwd);
          if(t<0){
            printf("lrmdir failed\n");
          }
          else{
            printf("removed %s\n", cwd);
          }
        }
      }



      //rm
      else if(!strncmp(line, "lrm", 3)){
        if(strlen(line)>4){
          if(line[4]!='/'){
            getcwd(cwd, 128);
            strcat(cwd, "/");
            sscanf(line, "lrmdir %s", ncwd);
            strcat(cwd, ncwd);

          }
          else{
            sscanf(line, "lrmdir %s", cwd);
          }
          t=remove(cwd);
          if(t<0){
            printf("lrm failed\n");
          }
          else{
            printf("removed %s\n", cwd);
          }
        }
        else{
          printf("incorrect command, use\nrm PATH\n");
        }
      }
      else if(!strncmp(line, "get", 3)){
        n = write(server_sock, line, MAX);
        printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

        n=read(server_sock, str, MAX);
        sscanf(str, "%d %s", nint, cwd);
        fstr = malloc(ntohl(nint) * sizeof(char));
        fp = fopen(cwd, "ab+");
        n = read(server_sock, fstr, ntohl(nint));
        fprintf(fp, "%s\n", fstr);
        fclose(fp);
        free(fstr);
      }
      else if(!strncmp(line, "put", 3)){
        n = write(server_sock, line, MAX);
        printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

        n=read(server_sock, str, MAX);
        sscanf(str, "%d %s", nint, cwd);
        fstr = malloc(ntohl(nint) * sizeof(char));
        fp = fopen(cwd, "ab+");
        n = read(server_sock, fstr, ntohl(nint));
        fprintf(fp, "%s\n", fstr);
        fclose(fp);
        free(fstr);
      }
    else{//else not local command, send to server
    // Send ENTIRE line to server
    n = write(server_sock, line, MAX);
    printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

    //first read message length
    n=read(server_sock, &nint, MAX);
    nint = ntohl(nint);
    printf("receiving %d bytes...\n", nint);
    str = malloc(nint * sizeof(char));
    n = read(server_sock, str, nint);
    printf("%s\n", str);
    //printf("client: read  n=%d bytes; echo=(\n%s)\n\n", nint, str);
    memset(str, '\0', nint);
    free(str);
  }
}
}
