/**
* @file comp_detection.h
* @brief Function prototypes and global variables
*
* @author Javier Pimentel
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Used for nanosleep()
#include <time.h>
// Used for threads.
#include <pthread.h>
//< Required to use sockets.
#include <sys/types.h>
#include <sys/socket.h>
// Contain the packet headers used
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <zlib.h>

/**
* Hold the host machines IP.
*/
#define SRC  "192.168.1.3"

/**
* Used to time stamp each icmp packet that is received.
*/
typedef struct{
    double sendTime; ///< Records the sending time.
    double recTime; ///< Records the time the icmp message was received.
} Rtt;

/**
* Array of type Rtt.
* Used by the sending and receiving thread to timestamp.
*/
Rtt *packetTime;

/**
* Holds all the command line arguments.
* Passed to the sending and receiving thread.
*/
struct args{
    char *dest;///< The Receiver’s IP Address
    int port;///< Port Number for UDP
    char *entro;///< Low or High Entropy (This will be a single character. Either “H” or “L”)
    int dataSize;///< The Size of the UDP Payload in the UDP Packet Train
    int nUdpPackets;///< The Number of UDP Packets in the UDP Packet Train
    int ttl;///< TTL for the UDP Packets
    int packetSpacing;///< The Inter-Packet Departure Time between Tail ICMP Packets (in milliseconds)
    int nTailICMP;///< The Number of Tail ICMP Packets
};

/**
* Lock for packetTime.
* Used to avoid a race condition when accessing packetTime as it is used
* by the sending and receiving thread.
*/
pthread_mutex_t lock;

/**
* Used to count the number of items in the packetTime array.
* Used by sendICMP() to keep track of the number of packets sent.
*/
int sentCount = 0;

/**
* @brief Sends all the packets
*
* Called by main to send icmp and upd packets. Opens a raw socket
* to be able to send the packets. Sleeps in between sending the
* tail icmp packets.
*
* @see sendICMP()
* @see sendUdpTrain()
* @param void* command line arguments
* @return void
*/
void *sendPackets(void*);

/**
*@brief Receives icmp messages.
*
* Called by main to receive icmp messages. Runs on its own thread.
* After all the messages are receives the RTT is calculated.
*
* @param void* Holds all the command line arguments.
* @return void
*/
void *recPackets(void*);

/**
* @brief IP checksum
* @return IP checksum as a unsigned short.
*/
unsigned short in_cksum(unsigned short * , int);

/**
* @brief Returns the current time as a double
* @return Current time as a double
*/
double get_time();

/**
*@brief Sends a icmp message
*
* Builds the ip and icmp header. Send the message to the detection
* specified in the command line argument. Called by sendPackets().
*
* @param socket Raw Socket
* @param args* Contains the command line arguments.
* @see sendPackets()
*/
void sendICMP(int socket,struct args*);

/**
*@brief Sends a train of udp packets.
*
* Uses the command line arguments to build the ip and udp header.
* If the arguments contain H it reads from /dev/urand and sets it
* as the payload. If L is contained it sends zeros as the payload.
*
* @param socket Raw socket used to send the packets.
* @param args* Command line arguments
*/
void sendUdpTrain(int socket,struct args*);
