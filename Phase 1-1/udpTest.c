
#include<stdio.h> //for printf
#include<string.h> //memset
#include<sys/socket.h>    //for socket ofcourse
#include<stdlib.h> //for exit(0);
#include<errno.h> //For errno - the error number
#include<netinet/udp.h>   //Provides declarations for udp header
#include<netinet/ip.h>

char dst_addr[20]= "127.0.0.1";
char src_addr[20] = "127.0.0.1";

void sendUdpTrain(int,int,int,int,int,char*,char*);
unsigned short in_cksum(unsigned short *, int);


int main(){
    
    //IP_HDRINCL is set because we are providing our own ip header
  //  setsockopt(mainSocket, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(int));

    //allocates memory for the array that will hold times
    //packetTime = (ttl*) malloc((sizeof ttl) * (numOfTailICMP + 1) );

    //           socket      n , size,ttl,port,entorp,dest
    sendUdpTrain(1, 1, 1000, 54, 2020, "low","127.0.0.1");
}
void sendUdpTrain(int Socket,int n,int size,int ttl,int desPort,char* entro,char *dest){

	int rawSocket;
    struct iphdr *iph;
    struct udphdr *udp;
    char *data;
    char packet[4096];
    memset(packet, 0, 4096);
    struct sockaddr_in connection;
     //ipv4
    connection.sin_family = AF_INET;
    connection.sin_port = 2222;
    //set the dest ip addr
    connection.sin_addr.s_addr = inet_addr(dest);
    //
    /*
     On Linux when setting the protocol as IPPROTO_RAW,
then by default the kernel sets the IP_HDRINCL option and thus does not prepend
its own IP header.
    */
    if ((rawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1)
    {
        perror("socket");
    }


    //allocates memory
    
    //set pos
    iph = (struct iphdr*) packet;
    udp = (struct udphdr*) (packet + sizeof(struct iphdr));
    data = packet + sizeof(struct iphdr) + sizeof(struct udphdr);
    strcpy(data, "look at me");
    //
    ///////////////////buidling ip header
    iph->ihl = 5;
    //ipv4
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data);
    int testing = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data);
    iph->id = htons(0);
    iph->frag_off = 0;
    iph->ttl = 200;
    //icmp message chaing from iPProto_icmp
    iph->protocol = IPPROTO_UDP;
    //returns an internet address
    iph->saddr = inet_addr(src_addr);
    iph->daddr = inet_addr(dest);
    //checksum
    iph->check = in_cksum((unsigned short *) iph, sizeof(struct iphdr));
    ////////////////////////////
    //need to att htons
    udp->source = htons(6666);
    udp->dest = htons(4222);
    int len = strlen(data);
    printf("%d \n",len);
    udp->len = htons(8 + 10);
    udp->check = 0;
    /////////////////

    //fill the data with random stuff
    //memset(&data, 2, sizeof data);

    //copies the data into the packet
  //  memcpy((packet + sizeof(ip) + sizeof(udp)), &data, len);

   
    if( sendto(rawSocket, packet, testing, 0, (struct sockaddr *)&connection,
            sizeof(connection)) == -1) {
        printf("error with sending\n");
    }
    else{

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