#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT "9876"
#define IP "127.0.0.1"

int main(){
    int mainSocket;
    struct addrinfo hints;
    struct addrinfo *serverInfo;
    char hello[] = "hello world!";
    int len;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    //getting addr info
    if(getaddrinfo(IP, PORT, &hints,&serverInfo) == -1){
        perror("wtf could not get info \n");
        exit(1);
    }
    //gets a socket
    if((mainSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) == -1){
        perror("wtf could not get a socket \n");
        exit(1);
    }
    len = strlen(hello);
    if(sendto(mainSocket, hello, len, 0, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1){
        perror("wtf could not send");
    }
    freeaddrinfo(serverInfo);
    close(mainSocket);
}