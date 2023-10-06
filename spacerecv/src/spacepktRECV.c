// $t@$h  QVLx Labs

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ccsds.h"
#include "cfe_sb.h"

#define BUFSIZE 2000

void printByteArray (char* byteArray, int size);
void unpackU32(uint8_t* inputBuffer, uint32_t* valueToUnpack, uint32_t* unpackingIndex);

void error(char *msg) {
	perror(msg);
	exit(1);
}

int main(int argc, char **argv) {
	int sockfd; /* socket */
	int portno; /* port to listen on */
	socklen_t clientlen; /* byte size of client's address */
	struct sockaddr_in serveraddr; /* server's addr */
	struct sockaddr_in clientaddr; /* client addr */
	struct hostent *hostp; /* client host info */
	char buf[ BUFSIZE ]; /* message buf */
	char *hostaddrp; /* dotted decimal host addr string */
	int optval; /* flag value for setsockopt */
	int n; /* message byte size */

	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	}
	portno = atoi(argv[1]);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");

	optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
			(const void *)&optval , sizeof(int));

	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	serveraddr.sin_port = htons((unsigned short)portno);

	if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) 
		error("ERROR on binding");

	clientlen = sizeof(clientaddr);
	int packetCounter = 0;	

	while (1) {
		bzero(buf, BUFSIZE);
		n = recvfrom(sockfd, buf, 30, 0, (struct sockaddr *) &clientaddr, &clientlen);

		printf("Received data size: %d bytes\n", n);
		printByteArray(buf, n);

		CFE_SB_Msg_t *ptr = (CFE_SB_Msg_t *)&buf;
		uint16 hdrSize = CFE_SB_MsgHdrSize(ptr);
		printf("Header Size: %d\n", hdrSize);
		CFE_SB_MsgId_t id = CFE_SB_GetMsgId(ptr);
		printf("ID: %d\n", id );
		uint16 length = CFE_SB_GetUserDataLength(ptr);
		printf("Data Length: %d\n", length);
		//id* tempRaw = CFE_SB_GetUserData(ptr);
		uint32 unpackedData[2];
		uint8* byteBuffer = (uint8*)malloc(n);
		int j = 0;
		for (int i = (n - 1); i >= 0; i--, j++) {
			byteBuffer[j] = buf[i];
		}

		int startIndex = 0;
                uint32 data2 = 0;
		for (int i = 0; i < 4; i++) {
                  *(&data2) <<= 8;
                  *(&data2) |= byteBuffer[startIndex + i];
		}
		startIndex = 4;
                uint32 data1 = 0;
		for (int i = 0; i < 4; i++) {
                  *(&data1) <<= 8;
                  *(&data1) |= byteBuffer[startIndex + i];
		}

		printf("data1: %d\n", data1);
		printf("data2: %d\n", data2);
		free(byteBuffer);
		packetCounter++;
		printf("PACKET COUNTER: %d\n", packetCounter);
	}
	return 0;
}

void printByteArray ( char* byteArray, int size )
{
	printf("packet: [");
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

void unpackU32(uint8_t* inputBuffer, uint32_t* valueToUnpack, uint32_t* unpackingIndex) {
	*valueToUnpack = 0;
	for (int i = 0; i < 4; i++) {
		*valueToUnpack <<= 8;
		*valueToUnpack |= inputBuffer[*unpackingIndex];
		(*unpackingIndex)++;
	}
}
