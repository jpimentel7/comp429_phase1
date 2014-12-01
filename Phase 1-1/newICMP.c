#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//used for nanosleep
#include <time.h>
//
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip.h>

#define DEST "192.168.1.1"
#define SRC  "192.168.1.3"
//input
char dst_addr[20]= "127.0.0.1";
char src_addr[20] = "192.168.1.3";

int datasize = 1000;
int numOfUdpPacket = 2;
int numOfTailICMP =9;
int portNum = 2000;
char *entropy = "low";
int ttl = 64;
int innerPacketLenght = 2;
//used to keep count of how many packets we get
int pCount = 0;
int totalPacket = 2+1+9;
//used to store packet send time
typedef struct{
    double sendTime;
    double recTime;
} Rtt;
Rtt *packetTime;
//used to count the number of items in the packetTime array
int sentCount = 0;
//fp
void *sendPackets();
unsigned short in_cksum(unsigned short *, int);
double get_time();
void sendICMP(int,char*);
// socket, how many,size,ttl, entropy,dest
void sendUdpTrain(int,int,int,int,int,char*,char*);

int main(){
    pthread_t sendingThread;
    //used to hold the values we received
    char *buff;
    int len;
    int recSocket;
    struct sockaddr_in connection;
    struct iphdr *recIpHeader;
    struct icmphdr *recIcmpHeader;
    //used to store times
    packetTime = malloc(sizeof(Rtt) * (1 + numOfTailICMP));
    //keeps track of which packets we got 
    int recCounter = 0;

    //starts a thread that send all the packets leaving the main thread free to listen
    if (pthread_create(&sendingThread,NULL,sendPackets,NULL))
    {
        perror("sending thread");
    }
    //used to hold the packet we get back
    buff = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr));
    len =sizeof(connection);
    //listen for in coming icmp packets
    if ((recSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
    {
        perror("socket");
    }
    //listen for in coming icmp packets
    while(1){
        if (recvfrom(recSocket, buff, sizeof(struct iphdr) + sizeof(struct icmphdr),
         0, (struct sockaddr *)&connection, &len) == -1)
        {
            perror("recv");
        }
        else{
            pCount++;
            recIpHeader = (struct iphdr*) buff;
            recIcmpHeader = (struct icmphdr*)(buff + sizeof(struct iphdr));
            //printf("The code we got back was :%d and the type was:%d\n",
              //  recIcmpHeader->code,recIcmpHeader->type);
            //checks the code and if its a repley the rtt is outputed 
            if(recIcmpHeader->code == 0){
                double diff = get_time() - packetTime[recCounter].sendTime;
                printf("The rtt was:%f \n",diff);
            }
            recCounter++;
            memset(buff,0,sizeof(buff));
        }
        if(pCount == totalPacket)
            break;
    }

    //waits for the other thread to finish
    if(pthread_join(sendingThread,NULL)){
        printf("wtf");
    }
}

void *sendPackets(){
    int rawSocket;
    /*
     On Linux when setting the protocol as IPPROTO_RAW,
    then by default the kernel sets the IP_HDRINCL option and thus does not prepend
    its own IP header.
    */
    if ((rawSocket = socket(AF_INET, SOCK_RAW, 255)) == -1)
    {
        perror("socket");
    }
    //sends the first icmp
    sendICMP(rawSocket,DEST);
    //socket, how many, size , ttl, dest port, dest, entropy
    sendUdpTrain(rawSocket,numOfUdpPacket,100,55,2989,DEST,"H");
    //sends n icmp packets every ms m seconds
    int millisec = 100;
    struct timespec req;
    req.tv_sec=0;
    req.tv_nsec=millisec * 1000000L;

    int i = 0;
    for(i=0;i<numOfTailICMP;i++){
        sendICMP(rawSocket,DEST);
        if(nanosleep(&req,(struct timespec*)NULL) == -1){
            perror("nanosleep:");
        }
    }
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
    ip->saddr = inet_addr(SRC);
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
    if(sendto(rawSocket, packet, ip->tot_len, 0, (struct sockaddr *)&connection, sizeof(connection)) == -1){
        perror("error with sending\n");
    }
    else{
        //
        Rtt temp;
        temp.sendTime = get_time();
        printf("the sending time is :%f \n",temp.sendTime);
        packetTime[sentCount] = temp;
        sentCount++;

    }

}
// socket, how many,size, entropy,dest
void sendUdpTrain(int rawSocket,int n,int size,int ttl,int desPort,char* dest,char *entro){
    struct iphdr *ip;
    struct udphdr *udp;
    char *data;
    char packet[4096];
    memset(packet, 0, 4096);

    struct sockaddr_in connection;
    //ipv4
    connection.sin_family = AF_INET;
    connection.sin_port = desPort;
    connection.sin_addr.s_addr = inet_addr(dest);
    //sets the headers position relative to each other
    ip = (struct iphdr*)packet;
    udp = (struct udphdr*)(packet + sizeof(struct iphdr));
    data = packet + sizeof(struct iphdr) + sizeof(struct udphdr);
    //testing
    //payload
    strcpy(data, "look at me");
    //Buidling ip header
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data);
    ip->id = htons(0);
    ip->frag_off = 0;
    ip->ttl = ttl;
    ip->protocol = IPPROTO_UDP;
    //returns an internet address
    ip->saddr = inet_addr(SRC);
    ip->daddr = inet_addr(dest);
    ip->check = in_cksum((unsigned short *) ip, sizeof(struct iphdr));
    //Building the upd header
    //web port lol
    udp->source = htons(80);
    udp->dest = htons(desPort);
    //the size of a udp header is 8
    udp->len = htons(8 + strlen(data));
    udp->check = 0;
    int i =0;
    for(i=0;i<n;i++) {
        if (sendto(rawSocket, packet, ip->tot_len, 0, (struct sockaddr *) &connection,
                sizeof(connection)) == -1) {
            printf("error with sending\n");
        }
    }
}

double get_time(){
    struct timeval tv;
    double d;
    gettimeofday (&tv, NULL);
    d = ((double) tv.tv_usec) / 1000000. + (unsigned long) tv.tv_sec;
    return d;
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