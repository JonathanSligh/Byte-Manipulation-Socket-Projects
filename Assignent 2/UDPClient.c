#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

short checkSum(char *msg, int length){
	short sum = 0;
	int i = 0;
	for (i; i < length; i++)
	{
		sum = sum + (unsigned short) (msg[i] & 0x00FF);
		sum = (sum & 0xFF) + (sum >> 8);
	}
	return sum;
}

int main(int argc, char *argv[])
{
	int sockfd;
	short receivedMessageLength = 0;
	short check = 0;
	//short numb = 0;
	short counter = 0;
	short l = 0;
	short ipCounter = 0;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

	if (argc < 5) {
		fprintf(stderr,"usage: talker hostname message\n");
		exit(1);
	}
	unsigned char msg[1024];
	unsigned char temp[10];
	/*int t1 = 46;
	memcpy(temp, &t1, sizeof(int));
	msg[0] = temp[0];
	int t2 = 60;
	memcpy(temp, &t2, sizeof(int));
	msg[1] = temp[0];*/
	msg[0] = 0x12;
	msg[1] = 0x34;
	int groupId = 7;
	memcpy(temp, &groupId, sizeof(int));
	msg[5] = temp[0];
	int requestId = atoi(argv[3]);
	memcpy(temp, &requestId, sizeof(int));
	msg[6] = temp[0];
	
	int messageLength = 7;
	int numHostNames = argc - 4;
	int i = 0;
	for(i; i < numHostNames; i++)
	{
		int hostNameLength = strlen(argv[4+i]);
		memcpy(temp, &hostNameLength, sizeof(int));
		msg[messageLength] = temp[0];
		messageLength++;
		int j = 0;
		for (j; j < strlen(argv[4+i]); j++)
		{
			msg[messageLength] = argv[4+i][j];
			messageLength++;
		}
	}
	memcpy(temp, &messageLength, sizeof(int));
	msg[2] = temp[1];
	msg[3] = temp[0];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}
	char hi[strlen(argv[3])];
	strcpy(hi, argv[3]);

	short sum = checkSum(msg, messageLength);
	msg[4] = ~(sum & 0xff);
	sum = ~(sum & 0xff);
	while(check == 0)
	{
		counter++;
		if (counter >= 7)
		{
			break;
		}
		if ((numbytes = sendto(sockfd, msg, messageLength, 0,
				 p->ai_addr, p->ai_addrlen)) == -1) {
			perror("talker: sendto");
			exit(1);
		}

		printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
		char buf[1024];
		receivedMessageLength = recvfrom(sockfd, buf , 1024, 0, p->ai_addr, &(p->ai_addrlen));
		printf("checkSum: %d\n", checkSum(buf, l));
		if (((short)buf[0] == 0x12) && ((short)buf[1] == 0x34))
		{
			char temp[2];
			temp[0] = buf[2];
			temp[1] = buf[3];
			l = (short)(buf[2]);
			l = l << 8;
			l = l + (short)buf[3];
			if (l >= 11)
			{
				if (checkSum(buf, l) == 255)
				{
					l = 0;
					for (l; l < argc - 4; l++)
					{
						printf("%s %d.%d.%d.%d\n", argv[l+4], (unsigned char)buf[ipCounter + 7], (unsigned char)buf[ipCounter+8], (unsigned char)buf[ipCounter+9], (unsigned char)buf[ipCounter+10]);
						ipCounter = ipCounter+4;
					}	
					counter = 6;
				}
			} 
		}
		printf("LENGTH: %d\n", receivedMessageLength); 
		
	}
	freeaddrinfo(servinfo);
	close(sockfd);
	return 0;
}
