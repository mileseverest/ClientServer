// This is the echo SERVER server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>

#define  MAX 256

// Define variables:
struct sockaddr_in  server_addr, client_addr, name_addr;
struct hostent *hp;

int  mysock, client_sock;              
int  serverPort;                     
int  r, length, n;              
int outFile;

void nullTerminatedCmd(char *cmd, char *arg)
{
    int size = strlen(arg);
    char out[256] = "";

    sprintf(out, "%d bytes: %s: %s\n", size, cmd, arg);
    printf("out: %s", out);
    n = write(client_sock, out, MAX);

}

void givenLengthCmd(char *line)
{
    short int size = 0;
    char buf[128] = "";
    read(client_sock, &size, 2);
    write(outFile, &size, 2);
    size = ntohs(size);
    printf("Size: %d\n", size);
    read(client_sock, buf, size);
    write(outFile, buf, size);
    printf("%s\n", buf);
}
int server_init(char *name)
{
   printf("******************** server init ********************\n");   

   //printf("1 : get and show server host info\n");
   hp = gethostbyname(name);
   if (hp == 0){
      printf("unknown host\n");
      exit(1);
   }
   printf("    hostname=%s  IP=%s\n",
               hp->h_name,  inet_ntoa(*(long *)hp->h_addr));
  
   //  create a TCP socket by socket() syscall
   //printf("2 : create a socket\n");
   mysock = socket(AF_INET, SOCK_STREAM, 0);
   if (mysock < 0){
      printf("socket call failed\n");
      exit(2);
   }

   //printf("3 : fill server_addr with host IP and PORT# info\n");
   // initialize the server_addr structure
   server_addr.sin_family = AF_INET;                  // for TCP/IP
   server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   // THIS HOST IP address  
   server_addr.sin_port = 0;//htons(50000);

   //printf("4 : bind socket to host info\n");
   // bind syscall: bind the socket to server_addr info
   r = bind(mysock,(struct sockaddr *)&server_addr, sizeof(server_addr));
   if (r < 0){
       printf("bind failed\n");
       exit(3);
   }

   //printf("5 : find out Kernel assigned PORT# and show it\n");
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
   printf("******************** init done ********************\n");
}


main(int argc, char *argv[])
{
   char *hostname;
   char line[MAX], readCommand[MAX] = "", command[MAX] = "",
        readPath[MAX] = "", path[MAX] = "";
   int port = 0;
   
   outFile = open("outfile.txt", O_WRONLY | O_CREAT, S_IRWXU);
   
 
   hostname = "localhost";
   server_init(hostname); 

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
       //write(outFile, line, strlen(line));
       
       if (n==0){
           printf("server: client died, server loops\n");
           close(client_sock);
           break;
      }
     
      sscanf(line, "%s %100c", readCommand, readPath);
      strcpy(command, readCommand);
      strcpy(path, readPath);
      memset(&readCommand[0], 0, MAX);
      memset(&readPath[0], 0, MAX);
       write(outFile, command, strlen(command));
       write(outFile, path, strlen(path));

      printf("Command: %s Path: %s\n", command, path);

      // show the line string
      //printf("server: read  n=%d bytes; line=[%s]\n", n, line);
      //printf("Command: %s\n", command);
      if (strcmp(command, "nullTerminatedCmd") == 0)
      {
          nullTerminatedCmd(command, path);
      }
      else if (strcmp(command, "givenLengthCmd") == 0)
      {
          givenLengthCmd(line);
      }
      else if (strcmp(command, "badIntCmd") == 0 || strcmp(command, "goodIntCmd") == 0)
      {
          int received_int = 0;
         
          n = read(client_sock, &received_int, sizeof(received_int));
          write(outFile, &received_int, 4);
          int outInt = ntohl(received_int);
          printf("Recieved: %d\n", outInt);
          n = write(client_sock, &outInt, sizeof(outInt));
      }
      
      if (strcmp(command, "kByteAtATimeCmd") == 0)
      {
          int received_int = 0, kinc = 0, op = 0;

          read(client_sock, &received_int, sizeof(received_int));
          write(outFile, &received_int, 4);
          int recInt = ntohl(received_int);
          printf("Recieved: %d\n", recInt);
          
          int rem = recInt % 1000;
          printf("REM: %d\n", rem);
          char buf[8000] = "";

          while (kinc < (recInt - rem))
          {
              memset(&buf[0], 0, 8000);
              op++;
              n = read(client_sock, buf, 8000);
              write(outFile, buf, 1000);
              kinc += (n/8); 
          }
          memset(&buf[0], 0, sizeof(buf));
    
          if (rem != 0)
          {      
              read(client_sock, buf, rem * 8);
              write(outFile, buf, 8);
              op++;
          }
          char out[128] = "";
          sprintf(out, "Operations: %d: %s: %d\n", op, command, recInt);
          write(client_sock, out, strlen(out));
      }

      if (strcmp(command, "byteAtATimeCmd") == 0)
      {
          int received_int = 0, binc = 0, op = 0;

          read(client_sock, &received_int, sizeof(received_int));
          write(outFile, &received_int, 4);
          int bInt = ntohl(received_int);
          printf("Recieved: %d\n", bInt);
          
          char buf[8] = "";

          while (binc < bInt)
          {
              memset(&buf[0], 0, 8);// sizeof(buf));
              op++;
              n = read(client_sock, buf, 8);// sizeof(buf));
              write(outFile, buf, 8);
              binc += (n/8); 
          }
          char out[128] = "";
          sprintf(out, "Operations: %d: %s: %d\n", op, command, bInt);
          write(client_sock, out, strlen(out));
      }
      // send the echo line to client 
     // n = write(client_sock, line, MAX);

      //printf("%s\n", line);
      printf("server: ready for next request\n");
    }
 }
}
