#ifndef HEADER_H_  /* Include guard */
#define HEADER_H_

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>

/* Constants */
#define MSS 1000
#define LOCALPORT 9999
#define LOCALADDRESS "127.0.0.1"
#define ETA "164.107.113.23"
#define BETA "164.107.113.18"
#define CLIENTTROLLPORT 10001
#define SERVERTROLLPORT 10003
#define TCPDSERVERPORT 10002
#define CBUFFERSIZE 64000 /* 64 KB buffer per spec */

/* Structs used */

/* tcpd to tcpd packet */
typedef struct Packet {
	char body[MSS];
	int bytes_to_read;
	int chksum;
	int packNo;
	//add additional vars here
} Packet;

/* tcpd to troll packet */
typedef struct MyMessage {
	struct sockaddr_in msg_header;
	struct Packet msg_pack;
	//int flag;	
} MyMessage;


//static char *cBuffer[CBUFFERSIZE];
/* client/server to tcpd packet */
//typedef struct tcpdHeader {
//	size_t maxData;
//	char body[MSS];
//	int bytes_to_read;
//} tcpdHeader;

#endif

#ifndef max
    #define max(a,b) ((a) > (b) ? (a) : (b))
#endif
