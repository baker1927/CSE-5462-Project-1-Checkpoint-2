int main(int argc, char *argv[]) {

	
	printf("%s\n\n", "Running on server machine...");	


	struct hostent *host; 					/* Hostname identifier */	
	fd_set selectmask;					/* Socket descriptor for select */	

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
	local_addr.sin_port = htons(TCPD_SERVER_LOCAL_PORT);
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
	network_addr.sin_port = htons(TCPD_SERVER_NETWORK_PORT);
	if (bind(network_sock, (struct sockaddr *)&network_addr, sizeof network_addr) < 0) {
		perror("Error binding to the network socket");
		exit(1);
	}

	/*----------------------^^^^^^^^------------------------------------------*/

	/*----------------------vvvvvvvv------------------------------------------*/
	/* Here we are establishing more addresses that we will be using */

	/* This is the address of the local FTPS */
	struct sockaddr_in destaddr;
	destaddr.sin_family = AF_INET;
	destaddr.sin_port = htons(LOCALPORT);
	destaddr.sin_addr.s_addr = inet_addr(LOCALADDRESS);

	/* This is the address of the client side TCPD */
	struct sockaddr_in client_tcpd_addr;
	client_tcpd_addr.sin_family = htons(AF_INET);
	client_tcpd_addr.sin_port = htons(9999);
	client_tcpd_addr.sin_addr.s_addr = inet_addr(ETA);

	/* This is the address of the server side TROLL*/
	struct sockaddr_in server_troll_addr;
	bzero ((char *)&server_troll_addr, sizeof server_troll_addr);
	server_troll_addr.sin_family = AF_INET;
	server_troll_addr.sin_port = htons(SERVER_TROLL_PORT);
	server_troll_addr.sin_addr.s_addr = inet_addr(BETA);

	/*----------------------^^^^^^^^------------------------------------------*/
		
		
	/* RECEIVE DATA */

	/* Initialize checksum table */		
	crcInit();

	/* Prepare descriptor*/
	FD_ZERO(&selectmask);
	FD_SET(network_sock, &selectmask);
	FD_SET(local_sock, &selectmask);	


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
    	    	
    			//Receive message into temp buffer
    			//compute checksum
    			//Add data to circular buffer and to auxList
    			//send ack for received data through server side troll to tcpd on client side
    			//send message data to FTPS

    		}
    		if (FD_ISSET(s2, &selectmask)) {
    			printf("Recieved data from FTPS.\n");

    			//compute sliding window movement
   	 		}
		}
	}

}