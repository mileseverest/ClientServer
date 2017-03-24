#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

/*#define nullTerminatedCmd  (1)
#define givenLengthCmd  (2)
#define badIntCmd  (3)
#define goodIntCmd  (4)
#define byteAtATimeCmd  (5)
#define kByteAtATimeCmd  (6)
#define noMoreCommands (0)
*/
typedef struct {
    unsigned cmd;
    char *arg;
} command;

void noMoreCommands()
{
    exit(1);
}

/*char *givenLengthCmd(char *line)
{
    short int size = strlen(line);
    char out[256] = "", *toke;

    toke = strtok(line, "\0");
    sprintf(out, "%d %s", size, toke);

    printf("%s\n", out);
    return out;
}*/

int badIntCmd(int server_sock, int path)
{
    int sendInt = atoi(path), receiveInt = 0;
    write(server_sock, &sendInt, sizeof(sendInt));
 
    read(server_sock, &receiveInt, sizeof(receiveInt));
 
    printf("Int: %d\n", receiveInt);

    return receiveInt;
}

int goodIntCmd(int server_sock, int path)
{
    int sendInt = htonl(atoi(path)), receiveInt = 0;
    write(server_sock, &sendInt, sizeof(sendInt));
 
    read(server_sock, &receiveInt, sizeof(receiveInt));
 
    printf("Int: %d\n", receiveInt);

    return receiveInt;
}
