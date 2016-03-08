/* 
* udpserver.c - A simple UDP echo server 
* usage: udpserver <port>
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 1024

/*
* error - wrapper for perror
*/
void error(char *msg) {
	perror(msg);
	exit(1);
}

int main(int argc, char **argv) {
	int sockfd; /* socket */
	int portno; /* port to listen on */
	int clientlen; /* byte size of client's address */
	struct sockaddr_in serveraddr; /* server's addr */
	struct sockaddr_in clientaddr; /* client addr */
	struct hostent *hostp; /* client host info */
	unsigned char buf[BUFSIZE]; /* message buf */
	char *hostaddrp; /* dotted decimal host addr string */
	char waitingAddr[500];
	int optval; /* flag value for setsockopt */
	int n; /* message byte size */
	int ifWaiting;
	ifWaiting = 0;

	/* 
* check command line arguments 
*/
	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	}
	portno = atoi(argv[1]);

	/* 
* socket: create the parent socket 
*/
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) 
	error("ERROR opening socket");

	/* setsockopt: Handy debugging trick that lets 
* us rerun the server immediately after we kill it; 
* otherwise we have to wait about 20 secs. 
* Eliminates "ERROR on binding: Address already in use" error. 
*/
	optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
	(const void *)&optval , sizeof(int));

	/*
* build the server's Internet address
*/
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)portno);

	/* 
* bind: associate the parent socket with a port 
*/
	if (bind(sockfd, (struct sockaddr *) &serveraddr, 
				sizeof(serveraddr)) < 0) 
	error("ERROR on binding");

	/* 
* main loop: wait for a datagram, then echo it
*/
	clientlen = sizeof(clientaddr);
	while (1) {

		/*
	* recvfrom: receive a UDP datagram from a client
	*/
		bzero(buf, BUFSIZE);
		n = recvfrom(sockfd, buf, BUFSIZE, 0,
		(struct sockaddr *) &clientaddr, &clientlen);
		if (n < 0)
		error("ERROR in recvfrom");

		/* 
	* gethostbyaddr: determine who sent the datagram
	*/
		hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
		sizeof(clientaddr.sin_addr.s_addr), AF_INET);
		if (hostp == NULL)
		error("ERROR on gethostbyaddr");
		hostaddrp = inet_ntoa(clientaddr.sin_addr);
		if (hostaddrp == NULL)
		error("ERROR on inet_ntoa\n");
		printf("server received datagram from %s (%s)\n", 
		hostp->h_name, hostaddrp);
		unsigned short val;
		val = (buf[0] << 8) | buf[1];
		short testport;
		testport = (buf[2] << 8) | buf[3];
		if (val != 42505 || (testport < 10010 + ((int)(buf[4])*5) || testport > 10014 + ((int)(buf[4])*5)) || strlen(buf) != 5)
		{
			unsigned char newMessage[5];
			newMessage[0] = (unsigned char)((((short)42505) >> 8) & 0xFF);
			newMessage[1] = (unsigned char)(((short)42505) & 0xFF);
			newMessage[2] = 0x07;
			newMessage[3] = 0x00;
			newMessage[4] = 0;
			if (val != 42505)
			{
				newMessage[4] = 0x01;  
			}
			if ((testport < 10010 + ((int)(buf[4])*5) || testport > 10014 + ((int)(buf[4])*5)))
			{
				newMessage[4] = newMessage[4] | 0x02;
			}
			if (strlen(buf) != 5)
			{
				newMessage[4] = newMessage[4] | 0x04;
			}
			n = sendto(sockfd, newMessage, 5, 0, 
			(struct sockaddr *) &clientaddr, clientlen);
			if (n < 0) 
			error("ERROR in sendto");
		}
		unsigned short port;
		if (ifWaiting == 0)
		{
			unsigned char newMessage[5];
			ifWaiting = 1;
			port = (buf[2] << 8) | buf[3];
			memcpy(waitingAddr, hostaddrp, 14);
			printf("%s", waitingAddr);
			newMessage[0] = buf[0];
			newMessage[1] = buf[1];
			newMessage[2] = 0x07;
			newMessage[3] = buf[2];
			newMessage[4] = buf[3];
			n = sendto(sockfd, newMessage, 5, 0, 
			(struct sockaddr *) &clientaddr, clientlen);
			if (n < 0) 
			error("ERROR in sendto");
		}
		else{
			unsigned char newMessage[19];
			newMessage[0] = buf[0];
			newMessage[1] = buf[1];
			newMessage[2] = waitingAddr[0];
			newMessage[3] = waitingAddr[1];
			newMessage[4] = waitingAddr[2];
			newMessage[5] = waitingAddr[3];
			newMessage[6] = waitingAddr[4];
			newMessage[7] = waitingAddr[5];
			newMessage[8] = waitingAddr[6];
			newMessage[9] = waitingAddr[7];
			newMessage[10] = waitingAddr[8];
			newMessage[11] = waitingAddr[9];
			newMessage[12] = waitingAddr[10];
			newMessage[13] = waitingAddr[11];
			newMessage[14] = waitingAddr[12];
			newMessage[15] = waitingAddr[13];
			newMessage[16] = (char)((port >> 8) & 0xFF);
			newMessage[17] = (char)((port) & 0xFF);
			newMessage[18] = 0x07;
			n = sendto(sockfd, newMessage, 19, 0, 
			(struct sockaddr *) &clientaddr, clientlen);
			if (n < 0) 
			error("ERROR in sendto");
			ifWaiting = 0;
		}
		printf("server received %d/%d bytes: %d\n", strlen(buf), n, val);
		
	}
}