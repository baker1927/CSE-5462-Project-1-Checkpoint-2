/*
/ Eric Olson and James Baker
/ CSE 5462 Network Programming
/ Project 1 - Checkpoint 1 - September 29, 2016
/ 
/ This file contains our tcpd daemon implementation
/ Note: /* CRC algorithm used from public domain implmentation from
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
		int ackSock;					
		MyMessage message; 						/* Packet sent to troll process */
		Packet packet;							/* Packet sent to server tcpd */
		//tcpdHeader tcpd_head;						/* Packet type from client */
		struct hostent *host; 						/* Hostname identifier */
		struct sockaddr_in trolladdr, destaddr, localaddr, clientaddr, tcpdaddr, clientack, masterAck;  /* Addresses */
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
		
		/* MASTER ACK SOCKET */
		/* This creates a socket to communicate with the local ftps process */
		if ((ackSock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("ackSock socket");
				exit(1);
		}
		bzero((char *)&masterAck, sizeof masterAck);
		masterAck.sin_family = AF_INET;
		masterAck.sin_addr.s_addr = inet_addr(ETA); /* let the kernel fill this in */
		masterAck.sin_port = htons(10050);
		if (bind(ackSock, (struct sockaddr *)&masterAck, sizeof masterAck) < 0) {
			perror("ack bind");
			exit(1);
		}

		/* SEND DATA TO TROLL */

		/* Initialize checksum table */	
		crcInit();

		/* Prepare for select */
		FD_ZERO(&selectmask);
		FD_SET(local_sock, &selectmask);
	
		//char ack[MSS] = "ACK";
		int packNo = 0;

		struct node *start,*temp;
        start = (struct node *)malloc(sizeof(struct node)); 
        temp = start;
        temp -> next = NULL;
		int current = 0;
		int next = 0;
		int ack = 0;
		
		/* Begin send loop */
		for(;;) {
			
				ack = 0;
				/* Wait for data on socket from cleint */
				if (FD_ISSET(local_sock, &selectmask)) {
				
				/* Receive data from ftpc and copy to local buffer */
				amtFromClient = recvfrom(local_sock, buffer, sizeof(buffer), MSG_WAITALL, NULL, NULL);
				printf("Received data from client.\n");
				
				/* Copy from local buffer to circular buffer */
				AddToBuffer(buffer); // Add to c buffer
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
				
				/* Copy payload from circular buffer to tcpd packet */
				bcopy(GetFromBuffer(), packet.body, bytesToSend); // removing from c buffer
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
	
				/* Send packet to troll */

				amtToTroll = sendto(troll_sock, (char *)&message, sizeof message, 0, (struct sockaddr *)&trolladdr, sizeof trolladdr);

				printf("Sent message to troll.\n\n");
				if (amtToTroll != sizeof message) {
					perror("totroll sendto");
					exit(1);
				}
				
				/* Get ack */
				recvfrom(ackSock, &ack, sizeof ack, MSG_WAITALL, NULL, NULL);				
				printf("**Ack Recieved: %d**\n\n", ack);
				
				/* *******WE JUST GOT AN ACK HERE MUH DUDE!!!! DO RTT SHIT HERE YO *********/
				
				
				//find the node that represents the packet that was just acked
				struct acked_node = findNode(head, ack.packNo);
				
				struct timespec endTime;
				clock_gettime(CLOCK_REALTIME, &endTime);

    			double elapsed = ( endTime.tv_sec - acked_node.time->.tv_sec )
  				+ ( endTime.tv_nsec - acked_node.time->.tv_nsec )
  				/ 1E9;
				
				calculate_rto(elapsed);
				printf("JUST CALCULATED THE RTO. NEW RTO IS: %f\n", rto);
				
				
				
				
				/* Send ack to ftpc after data written to buffer */
				sendto(local_sock, (char*)&ftpcAck, sizeof ftpcAck, 0, (struct sockaddr *)&clientack, sizeof clientack);
				/* For bookkeeping/debugging */
				total += amtToTroll;
				packNo = packNo + 1;
				
				/* maybe unnesscary but working so why not? */
				FD_ZERO(&selectmask);
				FD_SET(local_sock, &selectmask);
			} 

		/* Reset socket descriptor for select */
		FD_ZERO(&selectmask);
		FD_SET(local_sock, &selectmask);
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
		struct sockaddr_in trolladdr, localaddr, serveraddr, serverack, masterAck;    /* Addresses */
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

		/* TROLL ADDRESSS */
		struct sockaddr_in servertrolladdr;
		bzero ((char *)&servertrolladdr, sizeof servertrolladdr);
		servertrolladdr.sin_family = AF_INET;
		servertrolladdr.sin_port = htons(SERVERTROLLPORT);
		servertrolladdr.sin_addr.s_addr = inet_addr(BETA);
		
		/* MASTER ACK ADDRESS */
		bzero((char *)&masterAck, sizeof masterAck);
		masterAck.sin_family = AF_INET;
		masterAck.sin_addr.s_addr = inet_addr(ETA); /* let the kernel fill this in */
		masterAck.sin_port = htons(10050);
		
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
				
				/* SEND ACK TO CLIENT TCPD */
				int ack = 1;
				sendto(local_sock, &ack, ack, 0, (struct sockaddr *)&masterAck, sizeof masterAck);

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
				
				/* Forward packet body to server */
				amtToServer = sendto(local_sock, (char *)GetFromBuffer(), bytesToSend, 0, (struct sockaddr *)&destaddr, sizeof destaddr);
				if (amtToServer < 0) {
					perror("totroll sendto");
					/* To keep daemon running for grable demo */
					//exit(1);
				}
				printf("Copied data from buffer slot: %d\n", current);
				printf("Sent data to server.\n\n");
				
				/* Bookkeeping/Debugging */
				total += amtFromTcpd;
			}
			
			/* Reset decriptor */
			FD_ZERO(&selectmask);
			FD_SET(troll_sock, &selectmask);
		}
	}
}

