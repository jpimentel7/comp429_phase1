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

int main(int argc,char* argv[]) {
    int mainSocket;
    int dataStatus;
    //their ip
    struct sockaddr_storage their_addr;
    struct addrinfo hints;
    struct addrinfo *serverInfo;
    int sizeOfIP;
    int intChar = 0;
    char buff[40];
    char hello[] = "hello world!";
    int len = strlen(hello);
    int count =0;

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
    if((mainSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) == -1){
        perror("wtf could not get a socket \n");
        exit(1);
    }
    //binds to a port
    if(bind(mainSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)){
        perror("wtf could not connect");
        exit(1);
    }
    //going ot listen
    if(listen(mainSocket, 5) == -1){
        perror("wtf could not listen");
        exit(1);
    }

        int newSocket;
        //blocks the caller until a connection is present
        if((newSocket = accept(mainSocket, NULL, NULL)) == -1){
            perror("accept call failed \n");
        }
        while(recv(newSocket, &intChar, 10, 0) > 0){
            printf("we got:%d \n",intChar);
            close(newSocket);
            break;
        }
    close(mainSocket);
    ////ready to receive udp packets
    memset(&hints,0, sizeof hints);
    hints.ai_family = AF_UNSPEC; //ipv4 or 6
    hints.ai_socktype = SOCK_DGRAM; //udp
    hints.ai_flags = AI_PASSIVE; // fill in ip so we can leave the first parameter as null
    //gets the connection info
    if(getaddrinfo(IP, PORT, &hints,&serverInfo) == -1){
        perror("wtf could not get info \n");
        exit(1);
    }

    //gets a socket
    if((mainSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) == -1){
        perror("wtf could not get a socket \n");
        exit(1);
    }
    //binds to a port
    if(bind(mainSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)){
        perror("wtf could not connect");
        exit(1);
    }
    for(;;) {
        dataStatus = recvfrom(mainSocket, &buff, len, 0,NULL, NULL);
        if(dataStatus == -1){
            perror("look");
        }
        if (dataStatus > 0) {
            count++;
            printf("%s + %d \n", buff,count);
        }
    }

    close(mainSocket);

}