/**
* @file comp_detection.c
* @brief Sends icmp/udp packets to a IP address
*
* Sends an initial icmp message followed by a steam of upd packet. Then
* a train of icmp messages. When the messages return the RTT is calculated.
* Both the sending and receiving are done on different threads.
*
* @author Javier Pimentel
*/

#include "comp_detection.h"

/**
* @brief Calls the threads that send the packets.
*
* Creates two threads one to send and one to receive. Also checks to the number
* of comman line arguments. If there is less then 8 the program will give a warning
* then exit. After spawning the threads it waits for them to finish before exiting.
* before freeing up memory.
*
*/
int main(int argc ,char *argv[]){
    pthread_t sendingThread,receivingThread;
    struct args *perms;
    if(argc < 9)
        exit(0);
    perms = malloc(sizeof(struct args));
    //1. The Receiver’s IP Address
    perms->dest = argv[1];
    //2. Port Number for UDP
    perms->port = atoi(argv[2]);
    //3. Low or High Entropy (This will be a single character. Either “H” or “L”)
    perms->entro = argv[3];
    //4. The Size of the UDP Payload in the UDP Packet Train
    perms->dataSize = atoi(argv[4]);
    //5. The Number of UDP Packets in the UDP Packet Train
    perms->nUdpPackets = atoi(argv[5]);
    //6. TTL for the UDP Packets
    perms->ttl = atoi(argv[6]);
    //7. The Inter-Packet Departure Time between Tail ICMP Packets (in milliseconds)
    perms->packetSpacing = atoi(argv[7]);
    //8. The Number of Tail ICMP Packets
    perms->nTailICMP = atoi(argv[8]);

    //used to store times
    packetTime = malloc(sizeof(Rtt) * (1 + perms->nTailICMP));
    //starts a thread that send all the packets leaving the main thread free to listen
    if (pthread_create(&sendingThread,NULL,sendPackets,(void*)perms))
        perror("sending thread");
    //starts a thread that send all the packets leaving the main thread free to listen
    if (pthread_create(&receivingThread,NULL,recPackets,(void*)perms))
       perror("sending thread");

    //waits for the other thread to finish
    if(pthread_join(sendingThread,NULL))
        perror("joining sending thread");
    if(pthread_join(receivingThread,NULL))
        perror("joining receiving thread");

    pthread_mutex_destroy(&lock);
}

/**
* A raw socket is opened and receives all the icmp packets. All the repleys are time stamp.
* After all the packets have been received the RTT is calculated.
*/
void *recPackets(void *argu){
    //used to hold the values we received
    struct args *perms = (struct args*)argu;
    char *buff;
    int len,recSocket,packetNum;
    struct sockaddr_in connection;
    struct iphdr *recIpHeader;
    struct icmphdr *recIcmpHeader;
    Rtt temp;
    //keeps track of which packets we got
    int recCounter = 0;
    //used to hold the packet we get back
    buff = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr));
    len =sizeof(connection);
    //listen for in coming icmp packets
    if ((recSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
        perror("socket");
    //listen for in coming icmp packets
    packetNum = perms->nTailICMP + 1;
    while(recCounter != packetNum){
        if (recvfrom(recSocket, buff, sizeof(struct iphdr) + sizeof(struct icmphdr),
         0, (struct sockaddr *)&connection, &len) == -1){
            perror("recv");
        } else{

            recIpHeader = (struct iphdr*) buff;
            recIcmpHeader = (struct icmphdr*)(buff + sizeof(struct iphdr));
            temp.recTime = get_time();
            //printf("The code we got back was :%d and the type was:%d\n",
              //  recIcmpHeader->code,recIcmpHeader->type);
            //checks the code and if its a repley the rtt is outputed
            if(recIcmpHeader->code == 0){
                pthread_mutex_lock(&lock);
                packetTime[recCounter].recTime = get_time();
                pthread_mutex_unlock(&lock);
                //printf("The rtt was:%f \n",diff);
                recCounter++;
            }
            //restarts the buff
            memset(buff,0,sizeof(buff));
        }
    }
    int i;
    for(i=0;i<packetNum;i++){
        double diff = packetTime[i].recTime - packetTime[i].sendTime;
        printf("%s %f \n",perms->entro,diff);
    }

}

/**
* Opens a raw socket which is used to send udp/icmp packets. Runs on its own thread
* and is called from the main. Sends all the necessary packets.
*/
void *sendPackets(void *argu){
    struct args *perms = (struct args*)argu;
    int rawSocket;
    /*
     On Linux when setting the protocol as IPPROTO_RAW,
    then by default the kernel sets the IP_HDRINCL option and thus does not prepend
    its own IP header.
    */
    if ((rawSocket = socket(AF_INET, SOCK_RAW, 255)) == -1)
        perror("socket");
    //sends the first icmp message
    sendICMP(rawSocket,perms);
    sendUdpTrain(rawSocket,perms);
    //sends n icmp packets every ms
    int millisec = perms->packetSpacing;
    struct timespec req;
    req.tv_sec=0;
    req.tv_nsec=millisec * 1000000L;
    int i = 0;
    for(i=0;i<perms->nTailICMP;i++){
        sendICMP(rawSocket,perms);
        if(nanosleep(&req,(struct timespec*)NULL) == -1)
            perror("nanosleep:");
    }
}

/**
* Sends a ICMP message to dest using the socket
* pass to it. Also records the time after sending
* every packet.
*/
void sendICMP(int rawSocket , struct args *perms){
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
    ip->ttl = perms->ttl;
    ip->protocol = IPPROTO_ICMP;
    //returns an internet address
    ip->saddr = inet_addr(SRC);
    ip->daddr = inet_addr(perms->dest);
    ip->check = in_cksum((unsigned short *)ip , sizeof(struct iphdr));
    //Building up the icmp packet
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = random();
    icmp->un.echo.sequence = 0;
    icmp-> checksum = in_cksum((unsigned short *)icmp , sizeof(struct icmphdr));
    //ipv4
    connection.sin_family = AF_INET;
    connection.sin_addr.s_addr = inet_addr(perms->dest);
    //sending
    if(sendto(rawSocket, packet, ip->tot_len, 0, (struct sockaddr *)&connection, sizeof(connection)) == -1){
        perror("error with sending\n");
    } else{
        //
        Rtt temp;
        temp.sendTime = get_time();
        //printf("the sending time is :%f \n",temp.sendTime);
        pthread_mutex_lock(&lock);
        packetTime[sentCount] = temp;
        pthread_mutex_unlock(&lock);
        sentCount++;
    }
}

/**
* Sends N many udp packets to dest.
*/
void sendUdpTrain(int rawSocket ,struct args *perms){
    struct iphdr *ip;
    struct udphdr *udp;
    char *data;
    char packet[4096];
    memset(packet, 0, 4096);
    int size = perms->dataSize;
    struct sockaddr_in connection;
    //ipv4
    connection.sin_family = AF_INET;
    connection.sin_port = perms->port;
    connection.sin_addr.s_addr = inet_addr(perms->dest);
    //sets the headers position relative to each other
    ip = (struct iphdr*)packet;
    udp = (struct udphdr*)(packet + sizeof(struct iphdr));
    data = packet + sizeof(struct iphdr) + sizeof(struct udphdr);
    //testing
    if(perms->entro == "H") {
        FILE *ran;
        ran = fopen("/dev/urandom", "r");
        fread(data, 1, size, ran);
        fclose(ran);
    } else{
        char *temp;
        int len;
        int i;
        temp = malloc(sizeof(char) * size);
        for(i=0;i<size;i++)
            temp[i]='0';
        strcpy(data, temp);
    }
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data);
    ip->id = htons(0);
    ip->frag_off = 0;
    ip->ttl = perms->ttl;
    ip->protocol = IPPROTO_UDP;
    //returns an internet address
    ip->saddr = inet_addr(SRC);
    ip->daddr = inet_addr(perms->dest);
    ip->check = in_cksum((unsigned short *) ip, sizeof(struct iphdr));
    //Building the upd header
    //web port lol
    udp->source = htons(80);
    udp->dest = htons(perms->port);
    //the size of a udp header is 8 + size
    udp->len = htons(8 + strlen(data));
    udp->check = 0;
    int i =0;
    for(i=0;i<perms->nUdpPackets;i++) {
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