// The echo client client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include </home/mileseverest/Documents/College/Fall 2016/455/Project1/client/client.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAX 256

// Define variables
struct hostent *hp;              
struct sockaddr_in  server_addr; 

int server_sock, r;
int SERVER_IP, SERVER_PORT; 


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
  int n;
  char line[MAX], ans[MAX], input[MAX] = "", path[MAX] = "";
  command com;

  if (argc < 3){
     printf("Usage : client ServerName SeverPort\n");
     exit(1);
  }

  client_init(argv);
  // sock <---> server
  printf("********  processing loop  *********\n");
  while (1){
    printf("input a line : ");
    bzero(line, MAX);                // zero out line[ ]
    fgets(line, MAX, stdin);         // get a line (end with \n) from stdin

    line[strlen(line)-1] = 0;        // kill \n at end
    if (line[0]==0)                  // exit if NULL line
       exit(0);
                
    memset(&input[0], 0, sizeof(input));
    memset(&path[0], 0, sizeof(path));
    sscanf(line, "%s %100c", input, path);
   
    if (strcmp(input, "noMoreCommands") == 0)
    {
        noMoreCommands();
        //n = write(server_sock, line, MAX);
    }
    
    if (strcmp(input, "nullTerminatedCmd") == 0)
    {
        char in[256] = "";
        write(server_sock, line, strlen(line));
        read(server_sock, in, 256);

        printf("%s\n", in);
    }

    if (strcmp(input, "givenLengthCmd") == 0)
    {
        write(server_sock, input, MAX);
        short int size = htons(strlen(path));
        printf("Size: %d\n", htons(size)); 
        write(server_sock, &size, 2);
        write(server_sock, path, htons(size));
    }

    if (strcmp(input, "badIntCmd") == 0)
    {
        int receiveInt = 0;
        n = write(server_sock, input, MAX);
        
        receiveInt = badIntCmd(server_sock, path);   
    }

    if (strcmp(input, "goodIntCmd") == 0)
    {
        int receiveInt = 0;
        n = write(server_sock, input, MAX);
       
        receiveInt = goodIntCmd(server_sock, path);   
    }

    if (strcmp(input, "kByteAtATimeCmd") == 0)
    {
        int intSend = htonl(atoi(path)), i = 0, value = 0;
        n = write(server_sock, input, MAX);
        write(server_sock, &intSend, 4);
        char outChar[8000];
        outChar[0] = '\0';

        //printf("Output: %s\n", outChar);
        //printf("Send: %d\n", atoi(path));
        while (i < atoi(path))
        {
            if (i % 2 == 0)
                strcat(outChar, "00000001");
            
            else 
                strcat(outChar, "00000000");
            
            if (i % 1000 == 0)
            {
                int j = 0;
                write(server_sock, outChar, 8000);
                
                memset(&outChar[0], 0, sizeof(outChar));
            }
            
            i++;
        }
        
        if (i % 1000 != 0)
        {
            write(server_sock, outChar, strlen(outChar));
        }
        char in[128] = "";
        read(server_sock, in, sizeof(in));

        printf("%s\n", in);
    }

    if (strcmp(input, "byteAtATimeCmd") == 0)
    {
        int intSend = htonl(atoi(path)), i = 0, value = 0;
        n = write(server_sock, input, MAX);
        write(server_sock, &intSend, 4);
        char outChar[8];
        outChar[0] = '\0';

        //printf("Output: %s\n", outChar);
        //printf("Send: %d\n", atoi(path));
        while (i < atoi(path))
        {
            if (i % 2 == 0)
                write(server_sock, "00000001", 8);
            
            else 
                write(server_sock, "00000000", 8);
            i++;
        }

        char in[128] = "";
        read(server_sock, in, 128);
        printf("%s\n", in);
    }
    // Send ENTIRE line to server
    //printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

    // Read a line from sock and show it
   // n = read(server_sock, ans, MAX);
//    printf("%s\n", ans);
  }
}


