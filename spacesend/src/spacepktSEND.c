// $t@$h   QVLx Labs

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "ccsds.h"
#include "cfe_sb.h"

#define BUFSIZE 2000

#define h_addr h_addr_list[0]

void printByteArray(uint8_t* byteArray, int32_t size);
void error(char* msg);
void clearByteArray(uint8_t* byteString, uint32_t stringSize);
uint32_t packU32(uint8_t* packet, uint32_t value, uint32_t* packingIndex);

typedef struct{
    uint32 data1;
    uint32 data2;
} dataStruct;

typedef struct{
    CCSDS_TlmPkt_t header;
    dataStruct data; // this is the struct data format.
} testMessage;

void error(char *msg){
    perror(msg);
    exit(0);
}

int main(int argc, char **argv) {
    int sockfd; /* socket identifier */
    int portno; /* port number to send to */
    int n; /* not sure about this lol */
    struct sockaddr_in serveraddr; /* destination address information struct */
    int serverlen; /* size of serveraddr */
    struct hostent *server; /* DNS entry of ip address parameter */
    char *ipParam; /* ip address parameter */
    char buf[BUFSIZE];

    if (argc != 3) { /* quick check of command line arguments */
        fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
        exit(0);
    }
    ipParam = argv[1]; /* get ip address from command line argument */
    portno = atoi(argv[2]); /* get port number from command line argument */

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0) /* check for socket creation success */
    {
        error("ERROR opening socket");
    }

    server = gethostbyname(ipParam);
    if(server == NULL)
    {
        fprintf(stderr,"ERROR, no such host as %s\n", ipParam);
        exit(0);
    }

    bzero((char*) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverlen = sizeof(serveraddr);
    
    testMessage msg;
    (&msg)->data.data1 = 1;
    (&msg)->data.data2 = 2;
    CFE_SB_InitMsg(&msg, 1, sizeof((&msg)->data), TRUE);

    size_t totalSize = sizeof(CCSDS_TlmPkt_t) + sizeof(dataStruct);
    uint8_t serializedMsg[totalSize];
    memset(serializedMsg, 0, totalSize);
    memcpy(serializedMsg, &msg.header, sizeof(CCSDS_TlmPkt_t));
    memcpy(serializedMsg + sizeof(CCSDS_TlmPkt_t), &msg.data, sizeof(dataStruct));

    void* temp = (void *)&serializedMsg;
    printf("Sending %d bytes: ", totalSize);
    printByteArray(serializedMsg, totalSize);
    int loopIterations = 1;
    int i;
    for(i = 0; i < loopIterations; i++){
        n = sendto(sockfd, serializedMsg, totalSize, 0, (struct sockaddr *)&serveraddr, serverlen);
    }
    printf("sent %d packets.\n", i);
    return 0;
    }

void printByteArray(uint8_t* byteArray, int32_t size)
{
        printf("byteArray: [");
        int32_t i;
        for(i = 0; i < size; i++){
                if(i != (size - 1)){
                        printf("0x%02X, ", ((uint8_t *) byteArray)[i]);
                }
                else{
                        printf("0x%02X", ((uint8_t *) byteArray)[i]);
                }
        }
        printf("]\n");
}

void clearByteArray (uint8_t* byteString, uint32_t stringSize){
        memset(byteString, 0x00, stringSize);
}

uint32_t packU32(uint8_t* packet, uint32_t value, uint32_t* packingIndex)
{
	packet[*packingIndex] = (uint8_t)((value >> (uint32_t)(32 - 8)) & (uint32_t)0xff);
	(*packingIndex)++;
	packet[*packingIndex] = (uint8_t)((value >> (uint32_t)(32 - 16)) & (uint32_t)0xff);
	(*packingIndex)++;
	packet[*packingIndex] = (uint8_t)((value >> (uint32_t)(32 - 24)) & (uint32_t)0xff);
	(*packingIndex)++;
	packet[*packingIndex] = (uint8_t)((value >> (uint32_t)(32 - 32)) & (uint32_t)0xff);
	(*packingIndex)++;
	return *packingIndex;
}
