int main(int argc, char *argv[]) {
	
	printf("%s\n\n", "Running on client machine...");	

	/*----------------------vvvvvvvv------------------------------------------*/	
	/* Here we are creating a socket for sending and receiving messages between 
	processes on the same machine. */	

	int local_sock; 
	if ((local_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Error creating the local socket");
		exit(1);
	}

	struct local_addr;
	bzero((char *)&local_addr, sizeof local_addr);
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = INADDR_ANY; /* let the kernel fill this in */
	local_addr.sin_port = htons(TCPD_CLIENT_LOCAL_PORT);
	if (bind(network_sock, (struct sockaddr *)&local_addr, sizeof local_addr) < 0) {
		perror("Error binding to the local socket");
		exit(1);
	}

	/*----------------------^^^^^^^^------------------------------------------*/

	/*----------------------vvvvvvvv------------------------------------------*/
	/* Here we are creating a socket for receiving and sending between processes 
	on different machines*/

	int network_sock;
	if ((network_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Error creating the network socket");
		exit(1);
	}

	struct network_addr;
	bzero((char *)&network_addr, sizeof network_addr);
	network_addr.sin_family = AF_INET;
	network_addr.sin_addr.s_addr = INADDR_ANY; /* let the kernel fill this in */
	network_addr.sin_port = htons(TCPD_CLIENT_NETWORK_PORT);
	if (bind(network_sock, (struct sockaddr *)&network_addr, sizeof network_addr) < 0) {
		perror("Error binding to the network socket");
		exit(1);
	}

	/*----------------------^^^^^^^^------------------------------------------*/


	/*----------------------vvvvvvvv------------------------------------------*/
	/* Here we are establishing more addresses that we will be using */

	/* This is the address of the local FTPC */
	struct sockaddr_in ftpc_addr;
	bzero ((char *)&ftpc_addr, sizeof ftpc_addr);
	ftpc_addr.sin_family = AF_INET;
	ftpc_addr.sin_port = htons(LOCALPORT);
	ftpc_addr.sin_addr.s_addr = inet_addr(LOCALADDRESS);

	/* This is the address of the server side TCPD */
	struct sockaddr_in server_tcpd_addr;
	bzero ((char *)&server_tcpd_addr, sizeof server_tcpd_addr);
	server_tcpd_addr.sin_family = htons(AF_INET);
	server_tcpd_addr.sin_port = htons(9999);
	server_tcpd_addr.sin_addr.s_addr = inet_addr(BETA);

	/* This is the address of the server side TROLL*/
	struct sockaddr_in client_troll_addr;
	bzero ((char *)&client_troll_addr, sizeof client_troll_addr);
	client_troll_addr.sin_family = AF_INET;
	client_troll_addr.sin_port = htons(SERVER_TROLL_PORT);
	client_troll_addr.sin_addr.s_addr = inet_addr(BETA);

	/*----------------------^^^^^^^^------------------------------------------*/


	/* Initialize checksum table */		
	crcInit();

	fd_set selectmask;					/* Socket descriptor for select */	

	for(;;) {

		/* Prepare descriptor*/
		FD_ZERO(&selectmask);
		FD_SET(network_sock, &selectmask);
		FD_SET(local_sock, &selectmask);	

		int rv = select(FS_SETSIZE, &selectmask, NULL, NULL, NULL);

		if (rv == -1) {
    		perror("SELECT failed"); 
    		exit(1);
		} else {
    		// one or both of the descriptors have data
    		if (FD_ISSET(network_sock, &selectmask)) {
    			printf("Recieved data from the network.\n");
    	    	
    			//compute sliding window movement

    		}
    		if (FD_ISSET(local_sock, &selectmask)) {
    			printf("Recieved data from FTPS.\n");

    			//Receive message into temp buffer
    			int amt_from_ftpc =recvfrom(local_sock, (char*)temp, 
    				sizeof(temp), NULL, NULL, NULL);

    			//Add data to circular buffer
    			AddToBuffer(temp);
				current = getStart();
				next = getEnd();
				printf("Copied data to buffer slot: %d\n", current);

				//send ack up to ftpc
				int ftpc_ack = 1;
				sendto(local_sock, (char*)&ftpc_ack, sizeof(ftpc_ack), 0, 
					(struct sockaddr *)&ftpc_addr, sizeof(ftpc_addr));
				printf("Sent ack up to ftpc\n");

				//send message through client side troll to tcpd on server side
				sendto();

				//add a node to the auxillary list

    			//compute checksum
    			int chksum = crcFast((char *)&packet,sizeof(packet));
				printf("Checksum of data: %X\n", chksum);
    			
    			//send ack to FTPC
    			
   	 		}
		}
	}
}