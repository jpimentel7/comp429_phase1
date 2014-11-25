#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT "2222"
#define IP "127.0.0.1"
#define N 13

int main(int argc,char* argv[]) {
    int newSocket;
    struct addrinfo hints;
    struct addrinfo *serverInfo;
    char count[] = N;
    int len;
    int adder;

    memset(&hints,0, sizeof hints);
    hints.ai_family = AF_UNSPEC; //ipv4 or 6
    hints.ai_socktype = SOCK_STREAM; //tcp
    hints.ai_flags = AI_PASSIVE; // fill in ip so we can leave the first parameter as null
    //gets the connection info
    if(getaddrinfo(IP, PORT, &hints,&serverInfo) == -1){
        perror("wtf could not get info \n");
        exit(1);
    }

    //gets a socket
    if((newSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) == -1){
        perror("wtf could not get a socket \n");
        exit(1);
    }
    //establishes a connection or returns -1
    //blocking function
    if(connect(newSocket, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1){
        perror("wtf could not connect");
        exit(1);
    }
    //release the addr info
    freeaddrinfo(serverInfo);

    len = strlen(count);
    if(send(newSocket,&count,len,0) == -1){
        perror("could not send");
        exit(1);
    }
    close(newSocket);

}