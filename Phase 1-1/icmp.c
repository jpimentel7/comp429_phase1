#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT "9876"
#define IP "127.0.0.1"

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
uint16_t ip_checksum(void* vdata,size_t length) {
    // Cast the data pointer to one that can be indexed.
    char* data=(char*)vdata;

    // Initialise the accumulator.
    uint32_t acc=0xffff;
    size_t i=0;
    // Handle complete 16-bit blocks.
    for (i;i+1<length;i+=2) {
        uint16_t word;
        memcpy(&word,data+i,2);
        acc+=ntohs(word);
        if (acc>0xffff) {
            acc-=0xffff;
        }
    }

    // Handle any partial block at the end of the data.
    if (length&1) {
        uint16_t word=0;
        memcpy(&word,data+length-1,1);
        acc+=ntohs(word);
        if (acc>0xffff) {
            acc-=0xffff;
        }
    }

    // Return the checksum in network byte order.
    return htons(~acc);
}
int main(int argc, char **argv){
    struct addrinfo hints;
    struct addrinfo *serverInfo;
    char hello[] = "hello world!";
    int len;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_RAW;
    //getting addr info
    if(getaddrinfo(IP, NULL, &hints,&serverInfo) == -1){
        perror("wtf could not get info \n");
        exit(1);
    }

    int mainSocket = (AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(mainSocket == -1){
        printf("wtf");
    }
    //
//    int hdrincl=1;
//    if (setsockopt(mainSocket,IPPROTO_IP,IP_HDRINCL,&hdrincl,sizeof(hdrincl))==-1) {
//        printf("wtf time twol lol\n");
//    }
    //
    const size_t req_size=8;
    struct icmphdr req;
    req.type=8;
    req.code=0;
    req.checksum=0;
    req.un.echo.id=htons(rand());
    req.un.echo.sequence=htons(1);
    req.checksum= ip_checksum(&req,req_size);

    if (sendto(mainSocket,&req,req_size,0,serverInfo->ai_addr,serverInfo->ai_addrlen)==-1) {
        printf("that did not work\n");
    }
}
