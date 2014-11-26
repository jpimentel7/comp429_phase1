#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <string.h>
#include <unistd.h>

char dst_addr[20]= "127.0.0.1";
char src_addr[20] = "127.0.0.1";

unsigned short in_cksum(unsigned short *, int);

int main(){
    //ip header
    struct iphdr* ip;
    //icmp header
    struct icmphdr* icmp;
    //used to store everything
    char* packet;
    //raw socket
    int mainSocket;

    /*
    short            sin_family;   // e.g. AF_INET, AF_INET6
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
     */
    struct sockaddr_in connection;
    //
    int optval;

    //allocates memory
    packet = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr));

    //points ip and icmp to the "packet"
    ip = (struct iphdr*)packet;
    icmp = (struct icmphdr*) (packet + sizeof(struct iphdr));

    //building the ip header
    //length
    ip->ihl = 5;
    //ipv4
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr);
    ip->id = htons(0);
    ip->frag_off = 0;
    //time to live
    ip->ttl = 64;
    //icmp message
    ip->protocol = IPPROTO_ICMP;
    //returns an internet address
    ip->saddr = inet_addr(src_addr);
    ip->daddr = inet_addr(dst_addr);
    //checksum
    ip->check = in_cksum((unsigned short *)ip, sizeof(struct iphdr));

    //building the icmp message
    icmp->type = ICMP_ECHO;
    //echo request
    icmp->code = 0;
    icmp->un.echo.id = random();
    icmp->un.echo.sequence = 0;
    icmp-> checksum = in_cksum((unsigned short *)icmp, sizeof(struct icmphdr));

    //tries to open a raw socket
    if ((mainSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    //IP_HDRINCL is set because we are providing our own ip header
    setsockopt(mainSocket, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(int));
    //ipv4
    connection.sin_family = AF_INET;
    //set the dest ip addr
    connection.sin_addr.s_addr = inet_addr(dst_addr);

    //packet are now sent !
    sendto(mainSocket, packet, ip->tot_len, 0, (struct sockaddr *)&connection, sizeof(struct sockaddr));
    printf("Sent %d byte packet to %s\n", ip->tot_len, dst_addr);

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