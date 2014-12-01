#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//used for nanosleep
#include <time.h>
//threads
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip.h>


int ttl = 100;
char *src_addr = "127.0.0.1";

unsigned short in_cksum(unsigned short *, int);
void sendICMP(int,char*);
void *sendPackets(void *soc);

int main(){
    int mainSocket;
    pthread_t sendingThread;
    //used to hold the values we received
    char *buff;
    int len;
    int recSocket;
    struct sockaddr_in connection;
    connection.sin_family = AF_INET;
    //set the dest ip addr
    connection.sin_addr.s_addr = inet_addr("127.0.0.1");

    struct iphdr *recIpHeader;
    struct icmphdr *recIcmpHeader;
    /*
     On Linux when setting the protocol as IPPROTO_RAW,
    then by default the kernel sets the IP_HDRINCL option and thus does not prepend
    its own IP header.
    */
    if ((mainSocket = socket(AF_INET, SOCK_RAW, 255)) == -1)
    {
        perror("socket");
    }
    //
    if (pthread_create(&sendingThread,NULL,sendPackets,&mainSocket))
    {
        perror("sending thread");
    }
    
    //used to hold the packet we get back
    buff = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr));
    len =sizeof(connection);
    if ((recSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
    {
        perror("socket");
    }
    //listen for in coming icmp packets
    if (recvfrom(recSocket, buff, sizeof(struct iphdr) + sizeof(struct icmphdr),
     0, (struct sockaddr *)&connection, &len) == -1)
    {
        perror("recv");
    }
    else{
        recIpHeader = (struct iphdr*) buff;
        recIcmpHeader = (struct icmphdr*)(buff + sizeof(struct iphdr));
        printf("The code we got back was :%d and the type was:%d\n",
            recIcmpHeader->code,recIcmpHeader->type);
    }

//starts a thread that send all the packets leaving the main thread free to listen 
    if(pthread_join(sendingThread,NULL)){
        printf("wtf");
    }

}
void *sendPackets(void *soc){
    int rawSocket = *(int*)soc;
    //sends the first icmp
    int millisec = 200;
    struct timespec req;
    req.tv_sec=0;
    req.tv_nsec=millisec * 1000000L;
    if(nanosleep(&req,(struct timespec*)NULL) == -1){
            perror("nanosleep:");
        }
    sendICMP(rawSocket,"127.0.0.1");
    return NULL;
}
void sendICMP(int rawSocket, char *dest){
    struct iphdr* ip;
    struct icmphdr* icmp;
    char *packet;
    struct sockaddr_in connection;
    //allocates memory
    packet = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr));
    //points ip and icmp to the "packet"
    ip = (struct iphdr*)packet;
    icmp = (struct icmphdr*) (packet + sizeof(struct iphdr));
    //fills in the ip/imcp header
    //ip
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr);
    ip->id = htons(0);
    ip->frag_off = 0;
    ip->ttl = ttl;
    ip->protocol = IPPROTO_ICMP;
    //returns an internet address
    ip->saddr = inet_addr(src_addr);
    ip->daddr = inet_addr(dest);
    ip->check = in_cksum((unsigned short *)ip, sizeof(struct iphdr));
    //Building up the icmp packet
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = random();
    icmp->un.echo.sequence = 0;
    icmp-> checksum = in_cksum((unsigned short *)icmp, sizeof(struct icmphdr));
    //ipv4
    connection.sin_family = AF_INET;
    connection.sin_addr.s_addr = inet_addr(dest);
    //sending
    if(sendto(rawSocket, packet, ip->tot_len, 0, (struct sockaddr *)&connection, 
    	sizeof(connection)) == -1){
        perror("error with sending\n");
    }

}

unsigned short in_cksum(unsigned short *addr, int len)
{
    register int sum = 0;
    u_short answer = 0;
    register u_short *w = addr;
    register int nleft = len;
/*
* Our algorithm is simple, using a 32 bit accumulator (sum), we add
* sequential 16 bit words to it, and at the end, fold back all the
* carry bits from the top 16 bits into the lower 16 bits.
*/
    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }
/* mop up an odd byte, if necessary */
    if (nleft == 1)
    {
        *(u_char *) (&answer) = *(u_char *) w;
        sum += answer;
    }
/* add back carry outs from top 16 bits to low 16 bits */
    sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
    sum += (sum >> 16); /* add carry */
    answer = ~sum; /* truncate to 16 bits */
    return (answer);
}