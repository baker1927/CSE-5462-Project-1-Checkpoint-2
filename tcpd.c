/*
/ Eric Olson and James Baker
/ CSE 5462 Network Programming
/ Project 1 - Checkpoint 1 - September 29, 2016
/ 
/ This file contains our tcpd daemon implementation
/ Note: CRC algorithm used from public domain implmentation from
/ http://www.barrgroup.com/Embedded-Systems/How-To/CRC-Calculation-C-Code
*/

#include "header.h"
#include "crc/crc.c"
#include "crc/crc.h"

/* for lint */
void bzero(), bcopy(), exit(), perror();
double atof();
#define Printf if (!qflag) (void)printf
#define Fprintf (void)fprintf



/* main */
int main(int argc, char *argv[])
{
	/* Validate initial args */
	if (argc < 1) {
		fprintf(stderr, "%s\n", "There are not enough arguments.");
		exit(1);
	}
	
	/* Run on client side */
	if (atoi(argv[1]) == 1) {
		/* Validate input args */
		if (argc < 1) {
			fprintf(stderr, "%s\n", "There are not enough arguments. Proper use: tcpd flag");
			exit(1);
		}

		printf("%s\n\n", "Running on client machine...");

		int troll_sock;							/* a socket for sending messages to the local troll process */
		int local_sock; 						/* a socket to communicate with the client process */
		int tcpd_sock;					
		MyMessage message; 						/* Packet sent to troll process */
		Packet packet;							/* Packet sent to server tcpd */
		//tcpdHeader tcpd_head;						/* Packet type from client */
		struct hostent *host; 						/* Hostname identifier */
		struct sockaddr_in trolladdr, destaddr, localaddr, clientaddr, tcpdaddr, clientack;  /* Addresses */
		fd_set selectmask; 						/* Socket descriptor for select */
		int amtFromClient, amtToTroll, total = 0; 			/* Bookkeeping vars for sending */
		int chksum = 0;							/* Checksum */
		char buffer[MSS] = {0};
		int ftpcAck = 1;
		/* TROLL ADDRESSS */
		/* this is the addr that troll is running on */

		//if ((host = gethostbyname(argv[2])) == NULL) {
		//	printf("Unknown troll host '%s'\n",argv[2]);
		//	exit(1);
		//}  
		bzero ((char *)&trolladdr, sizeof trolladdr);
		trolladdr.sin_family = AF_INET;
		//bcopy(host->h_addr, (char*)&trolladdr.sin_addr, host->h_length);
		trolladdr.sin_port = htons(CLIENTTROLLPORT);
		trolladdr.sin_addr.s_addr = inet_addr(ETA);

		/* DESTINATION ADDRESS */
		/* This is the destination address that the troll will forward packets to */

		//if ((host = gethostbyname(argv[4])) == NULL) {
		//	printf("Unknown troll host '%s'\n",argv[4]);
		//	exit(1);
		//} 
		//bzero ((char *)&destaddr, sizeof destaddr);
		destaddr.sin_family = htons(AF_INET);
    		//bcopy(host->h_addr, (char*)&destaddr.sin_addr, host->h_length);
		destaddr.sin_port = htons(TCPDSERVERPORT);
		destaddr.sin_addr.s_addr = inet_addr(BETA);
		
		/* Client ack address */
		bzero ((char *)&clientack, sizeof clientack);
		clientack.sin_family = AF_INET;
		//bcopy(host->h_addr, (char*)&clientack.sin_addr, host->h_length);
		clientack.sin_port = htons(10010);
		clientack.sin_addr.s_addr = inet_addr(ETA);

		/* SOCKET TO TROLL */
		/* This creates a socket to communicate with the local troll process */

		if ((troll_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("totroll socket");
			exit(1);
		}
		bzero((char *)&localaddr, sizeof localaddr);
		localaddr.sin_family = AF_INET;
		localaddr.sin_addr.s_addr = INADDR_ANY; /* let the kernel fill this in */
		localaddr.sin_port = 0;					/* let the kernel choose a port */
		if (bind(troll_sock, (struct sockaddr *)&localaddr, sizeof localaddr) < 0) {
			perror("client bind");
			exit(1);
		}

		/* SOCKET TO CLIENT */
		/* This creates a socket to communicate with the local troll process */

		if ((local_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("client socket");
			exit(1);
		}
		bzero((char *)&clientaddr, sizeof clientaddr);
		clientaddr.sin_family = AF_INET;
		clientaddr.sin_addr.s_addr = inet_addr(LOCALADDRESS); /* let the kernel fill this in */
		clientaddr.sin_port = htons(LOCALPORT);
		if (bind(local_sock, (struct sockaddr *)&clientaddr, sizeof clientaddr) < 0) {
			perror("client bind");
			exit(1);
		}

		/*-------------------------------V------------------------------------*/

		//Will become uneccessary

		//Create a socket to listen for ACK messages from the server side tcpd on
		int rcvr_sock;
		if((rcvr_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("rcvr_sock socket");
			exit(1);
		}
		//bind information to this socket
		struct sockaddr_in client_ack_addr;
		bzero((char *)&client_ack_addr, sizeof client_ack_addr);
		client_ack_addr.sin_family = AF_INET;
		client_ack_addr.sin_addr.s_addr = inet_addr(LOCALADDRESS); /* let the kernel fill this in */
		client_ack_addr.sin_port = htons(9898);
		if (bind(local_sock, (struct sockaddr *)&client_ack_addr, sizeof client_ack_addr) < 0) {
			perror("client_ack_addr bind");
			exit(1);
		}

		/*----------------------------------^---------------------------------*/

		/* SEND DATA TO TROLL */

		/* Initialize checksum table */	
		crcInit();

		/* Prepare for select */
		FD_ZERO(&selectmask);
		FD_SET(local_sock, &selectmask);

		/*---------------------------------V----------------------------------*/

		//Will become uneccessary
		FD_SET(rcvr_sock, &selectmask);

		/*--------------------------------^-----------------------------------*/
	
		//char ack[MSS] = "ACK";
		int packNo = 0;

		struct node *start,*temp;
        start = (struct node *)malloc(sizeof(struct node)); 
        temp = start;
        temp -> next = NULL;
		int current = 0;
		int next = 0;
		
		/* Begin send loop */
		for(;;) {
			
			/* Wait for data on socket from cleint */
			if (FD_ISSET(local_sock, &selectmask)) {
					
				/* Receive data from ftpc and copy to local buffer */
				amtFromClient = recvfrom(local_sock, buffer, sizeof(buffer), MSG_WAITALL, NULL, NULL);
				printf("Received data from client.\n");
				
				/* Copy from local buffer to circular buffer */
				AddToBuffer(buffer);
				current = getStart();
				next = getEnd();
				printf("Copied data to buffer slot: %d\n", current);
				
				/* Update aux list info */
				insertNode(temp, current, next, packNo, amtFromClient, 0, 0);
				
				/* Node to get info on current buffer slot */
				struct node *ptr;
				ptr = (struct node *)malloc(sizeof(struct node));
				ptr -> next = NULL;
				ptr = findNode(temp, current);
				int bytesToSend = ptr->bytes;
				
				/* Send ack to ftpc after data written to buffer */
				sendto(local_sock, (char*)&ftpcAck, sizeof ftpcAck, 0, (struct sockaddr *)&clientack, sizeof clientack);
				
				/* Copy payload from circular buffer to tcpd packet */
				bcopy(GetFromBuffer(), packet.body, bytesToSend);
				printf("Copied data from buffer slot: %d\n", current);
				
				/* Prepare packet */
				packet.bytes_to_read = bytesToSend;
				packet.chksum = 0;
				packet.packNo = packNo;
					
				/* Calculate checksum */				
				chksum = crcFast((char *)&packet,sizeof(packet));
				printf("Checksum of data: %X\n", chksum);
	
				/* Attach checksum to troll packet */
				/* This is checksum with chksum zerod out. Must do same on rec end */
				packet.chksum = chksum;
	
				/* Prepare troll wrapper */
				message.msg_pack = packet;
				message.msg_header = destaddr;

				/*-------------------------------v----------------------------*/
				
				//get time
				//store it in the auxList node
				//In the future, start timer for this packet with value RTO

				/*------------------------------^-----------------------------*/
	
				/* Send packet to troll */
				amtToTroll = sendto(troll_sock, (char *)&message, sizeof message, 0, (struct sockaddr *)&trolladdr, sizeof trolladdr);
				printf("Sent message to troll.\n\n");
				if (amtToTroll != sizeof message) {
					perror("totroll sendto");
					exit(1);
				}
	
				/* For bookkeeping/debugging */
				total += amtToTroll;
				packNo = packNo + 1;
				
				/* maybe unnesscary but working so why not? */
				FD_ZERO(&selectmask);
				FD_SET(local_sock, &selectmask);
				/*--------------------------------v---------------------------*/

				//Will become uneccessary
				FD_SET(rcvr_sock, &selectmask);
		
				/*---------------------------------^--------------------------*/
		
			}

			/*----------------------------------v-----------------------------*/
			
			//This will become unnecessary

			//we are receiving an ACK from the receiver
			if (FD_ISSET(rcvr_sock, &selectmask)) {
				printf("RECEIVED AN ACK ON THE CLIENT SIDE TCPD\n");
				//printf("Received an ACK for packet %d", ...);
				//get current time from corresponding auxList node
				//calculate time elapsed
				//calculate_rto();
				//printf("The RTO has been updated to %d", ...);
				//delete auxList node for that packet


				//In the future, we will also update the sliding window here
			}

			/*--------------------------------^-------------------------------*/ 

		/* Reset socket descriptor for select */
		FD_ZERO(&selectmask);
		FD_SET(local_sock, &selectmask);

		/*-------------------------------------v------------------------------*/

		//Will become uneccessary
		FD_SET(rcvr_sock, &selectmask);
		
		/*---------------------------------------^----------------------------*/
		}
		
	/* Run on server side */
	} else if (atoi(argv[1]) == 0) {

		/* Validate args */
		if (argc < 1) {
			fprintf(stderr, "%s\n", "There are not enough arguments. Proper use: tcpd flag");
			exit(1);
		}

		printf("%s\n\n", "Running on server machine...");		

		int troll_sock;						/* a socket for sending messages and receiving responses */
		int local_sock, ackSock; 					/* a socket to communicate with the client process */
		MyMessage message, clientMessage; 					/* recieved packet from remote troll process */
		Packet clientPacket;		
		struct sockaddr_in trolladdr, localaddr, serveraddr, serverack;    /* Addresses */
		struct hostent *host; 					/* Hostname identifier */
		fd_set selectmask;					/* Socket descriptor for select */
		int amtFromTcpd, amtToServer, len, total, amtToTcpd = 0; 		/* Bookkeeping vars */
		int chksum, recv_chksum = 0;						/* Checksum */
		int ftpsAck = 1;					/* ftps ack */
		
		/* SOCKET FROM TROLL */
		/* This is the socket to recieve from the troll running on the client machine */
		if ((troll_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("fromtroll socket");
			exit(1);
		}
		bzero((char *)&localaddr, sizeof localaddr);
		localaddr.sin_family = AF_INET;
		localaddr.sin_addr.s_addr = INADDR_ANY; /* let the kernel fill this in */
		localaddr.sin_port = htons(TCPDSERVERPORT);
		if (bind(troll_sock, (struct sockaddr *)&localaddr, sizeof localaddr) < 0) {
			perror("client bind");
			exit(1);
		}
	
		/* SOCKET TO SERVER */
		/* This creates a socket to communicate with the local troll process */
		if ((local_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("client socket");
			exit(1);
		}
		
		/* FTPS ACK SOCKET */
		/* This creates a socket to communicate with the local ftps process */
		if ((ackSock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("ackSock socket");
				exit(1);
		}
		bzero((char *)&serverack, sizeof serverack);
		serverack.sin_family = AF_INET;
		serverack.sin_addr.s_addr = inet_addr(BETA); /* let the kernel fill this in */
		serverack.sin_port = htons(10021);
		if (bind(ackSock, (struct sockaddr *)&serverack, sizeof serverack) < 0) {
			perror("ack bind");
			exit(1);
		}
		
		/* ADDRESS TO CONNECT WITH THE SERVER */
		struct sockaddr_in destaddr;
		destaddr.sin_family = AF_INET;
		destaddr.sin_port = htons(LOCALPORT);
		destaddr.sin_addr.s_addr = inet_addr(LOCALADDRESS);

		/* ADDRESS OF CLIENT TCPD */
		struct sockaddr_in clientaddr;
		clientaddr.sin_family = htons(AF_INET);
		clientaddr.sin_port = htons(9999);
		clientaddr.sin_addr.s_addr = inet_addr(ETA);

		/*---------------------------------V----------------------------------*/

		//This will be uneccessary. Will just send acks to the one socket on
		//the client side. No need for a separate socket

		//The information for the socket that ack messages will be sent to
		/*ADDRESS OF CLIENT TCPD ACK SOCKET*/
		struct sockaddr_in clientaddr_ack;
		clientaddr.sin_family = htons(AF_INET);
		clientaddr.sin_port = htons(9898);
		clientaddr.sin_addr.s_addr = inet_addr(ETA);

		//create a socket for sending ACKS directly to the client tcpd
		//In the future we will need to send acks to troll instead
		int cli_tcpd_sock;
		if((cli_tcpd_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("cli_tcpd_sock socket");
			exit(1);
		}

		/*----------------------------------^---------------------------------*/

		/* TROLL ADDRESSS */
		struct sockaddr_in servertrolladdr;
		bzero ((char *)&servertrolladdr, sizeof servertrolladdr);
		servertrolladdr.sin_family = AF_INET;
		servertrolladdr.sin_port = htons(SERVERTROLLPORT);
		servertrolladdr.sin_addr.s_addr = inet_addr(BETA);
		
		/* RECEIVE DATA */

		/* Initialize checksum table */		
		crcInit();

		/* Prepare descriptor*/
		FD_ZERO(&selectmask);
		FD_SET(troll_sock, &selectmask);

		struct node *start,*temp;
        start = (struct node *)malloc(sizeof(struct node)); 
        temp = start;
        temp -> next = NULL;
		int current = 0;
		int next = 0;
	
		/* Begin recieve loop */
		for(;;) {
		
			/* If data is ready to be recieved from troll on client machien */
			if (FD_ISSET(troll_sock, &selectmask)) {
				
				/* length of addr for recieve call */
				len = sizeof trolladdr;
	
				/* read in one packet from the troll */
				amtFromTcpd = recvfrom(troll_sock, (char *)&message, sizeof message, MSG_WAITALL,
					(struct sockaddr *)&trolladdr, &len);
				if (amtFromTcpd < 0) {
					perror("fromtroll recvfrom");
					exit(1);
				}

				printf("Recieved data from troll.\n");
				
				/* Prepare troll wrapper */
				//bcopy(&recvMessage, &clientPacket.body, sizeof(clientPacket.body));
				//clientPacket.packNo = message.msg_pack.packNo;
				//clientMessage.msg_pack = clientPacket;
				//clientMessage.msg_header = clientaddr;
				
				/* Send packet to troll */
				//amtToTcpd = sendto(troll_sock, (char *)&clientMessage, sizeof clientMessage, 0, (struct sockaddr *)&servertrolladdr, sizeof servertrolladdr);
				//usleep(1000);

				/* get checksum from packet */
				recv_chksum = message.msg_pack.chksum;

				/* zero checksum to make equal again */
				message.msg_pack.chksum = 0;

				/* Calculate checksum of packet recieved */
				chksum = crcFast((char *)&message.msg_pack,sizeof(message.msg_pack));
				printf("Checksum of data: %X\n", chksum);

				/* Compare expected checksum to one caluclated above. Print Error if conflict. */
				if (chksum != recv_chksum) {
					printf("CHECKSUM ERROR: Expected: %X Actual: %X\n", recv_chksum, chksum);
				}

				/* Add body to circular buffer */
				AddToBuffer(message.msg_pack.body);
				current = getStart();
				next = getEnd();
				printf("Copied data to buffer slot: %d\n", current);
				insertNode(temp, current, next, 0, message.msg_pack.bytes_to_read, 0, 0);
				
				/* Node to get info on current buffer slot */
				struct node *ptr;
				ptr = (struct node *)malloc(sizeof(struct node));
				ptr -> next = NULL;
				ptr = findNode(temp, current);
				int bytesToSend = ptr->bytes;
				
				/* TODO send ack to client tcpd here */

				/*-----------------------------v------------------------------*/

				//This section will need to be changed to send through the
				//server side troll

				//create an ACK response packet
				Packet ack_pack;
				bzero ((char *)&ack_pack, sizeof ack_pack);
				ack_pack.packNo = message.msg_pack.packNo;

				//Send to the client's tcpd (directly for now)
				int tempAmt = sendto(cli_tcpd_sock, (char*)&ack_pack, sizeof(ack_pack), 0, (struct sockaddr *)&clientaddr_ack, sizeof(clientaddr_ack));
				if (tempAmt < 0) {
					perror("ack to client tcpd sendto");
					exit(1);
				}
				printf("JUST SENT THE ACK TO CLIENT TCPD\n");
				 

				/*-------------------------------^----------------------------*/

				/* Forward packet body to server */
				amtToServer = sendto(local_sock, (char *)GetFromBuffer(), bytesToSend, 0, (struct sockaddr *)&destaddr, sizeof destaddr);
				if (amtToServer < 0) {
					perror("totroll sendto");
					/* To keep daemon running for grable demo */
					//exit(1);
				}
				printf("Copied data from buffer slot: %d\n", current);
				printf("Sent data to server.\n\n");
				
				/* Get ack from ftps */
				recvfrom(ackSock, &ftpsAck, sizeof(ftpsAck), MSG_WAITALL, NULL, NULL);
				//printf("Ack Recieved: %d\n\n", ftpsAck);

				/* Bookkeeping/Debugging */
				total += amtFromTcpd;
			}
			
			/* Reset decriptor */
			FD_ZERO(&selectmask);
			FD_SET(troll_sock, &selectmask);
		}
	}
}

