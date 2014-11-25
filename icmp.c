#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include <zlib.h>
#include <stdio.h>


#define BUFSIZE 1500


char sendbuf[BUFSIZE];
int datalen = 56; // # of bytes of data following the icmp header
char *host;
int nsent = 0;
pid_t pid;
int socketfd;
////
struct addrinfo *serverInfo;
////


//fucntion proto
unsigned short
in_cksum(unsigned short *addr,int len){
    int nleft = len;
    int sum =0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while(nleft > 1){
        sum += *w++;
        nleft -= 2;
    }

    if(nleft == 1){
        *(unsigned char *) (&answer) = *(unsigned char *) w;
        sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return (answer);
}
/////

int main(int argc, char **argv){
    //get input from the command line
    struct addrinfo hints;

    ////////
    memset(&hints, 0, sizeof hints);
    hints.ai_flags = AI_CANONNAME;
    hints.ai_family = 0;
    hints.ai_socktype = 0;
    if(getaddrinfo("127.0.0.1",NULL,&hints,&serverInfo) == -1){
        perror("wtf could not get info \n");
    }

    /////// creates the icmp
    pid = getpid();
    int len;
    struct icmp *icmp;
    icmp = (struct icmp *) sendbuf;
    icmp->icmp_type = ICMP_ECHO; //echo
    icmp->icmp_code = 0;
    icmp->icmp_id = pid;
    icmp->icmp_seq = nsent++;
    gettimeofday((struct timeval *)icmp->icmp_data, NULL);
    len = 8 + datalen;
    icmp->icmp_cksum = 0;
    icmp->icmp_cksum = in_cksum((u_short *)icmp, len);
    ////////
    ///// creates the raw socket
    int size = 60 * 1024;
    //serverInfo->ai_addr->sa_family
    socketfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(socketfd == -1){
        printf("wtf look at me ");
    }
    //trying SO_SNDBUF instead of rcvbuf
    if(setsockopt(socketfd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) == -1){
        printf("i didnt work");
    };
    ///////
    if((sendto(socketfd, sendbuf, len, 0, serverInfo->ai_addr, serverInfo->ai_addrlen)) == -1){
        printf("wtf look at me first");
    }



}
