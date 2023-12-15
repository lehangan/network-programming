#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h> 
#include <sys/time.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>

// #include "file_handle.h"

#define PORT 5500   /* Port that will be opened */ 
#define BACKLOG 20   /* Number of allowed connections */
#define BUFF_SIZE 1024

typedef struct MSG{
	int opcode;
	int length;
	char payload[BUFF_SIZE];
}MSG;

void encodeCeasar(char *text, int s){
	int n = strlen(text);
	for (int i = 0; i < n ; i++) {
		if (isalpha(text[i])) {
			char base = isupper(text[i]) ? 'A' : 'a';
			text[i] = (text[i] - base + s) % 26 + base;
		}
	}
}

void delete_file(char* filename) {
	if (remove(filename) == 0) { 
		printf("Deleted '%s' successfully.\n", filename); 
	} else { 
		printf("Unable to delete '%s'\n", filename);
	}
}

void handle_file(char* content, int opmode, int key) {
	if(opmode == 0){
		encodeCeasar(content,key); // encode
	}
	else if(opmode == 1){
		encodeCeasar(content, 26-key); // decode
	}
	else {
		printf("%d Not do anthing\n" , opmode);
	}
}

void MSGtobuffer(char* buffer, MSG* msg){
	// opcode: 1 bytes
	memcpy(buffer, &msg->opcode, sizeof(char));
	buffer += sizeof(char); // Move buffer pointer

	// length: 2 bytes
	uint16_t netw_length = htons(msg->length);	// Host to Network
	memcpy(buffer, &netw_length, sizeof(uint16_t));
	buffer += sizeof(uint16_t); // Move buffer pointer

	// payload: (length) bytes
	memcpy(buffer, msg->payload, msg->length);
}

void buffertoMSG(char* buffer, MSG* msg){
	// opcode: 1 bytes
	memcpy(&msg->opcode, buffer, sizeof(char));
	buffer += sizeof(char);		// Move buffer pointer

	// length: 2 bytes
	uint16_t netw_length;
	memcpy(&netw_length, buffer, sizeof(uint16_t));	
	buffer += sizeof(uint16_t);	// Move buffer pointer
	msg->length = ntohs(netw_length); // network to host short

	// payload: (length) bytes
	strncpy(msg->payload, buffer, msg->length);
}

char buff[1100];
const int EXTRA_SIZE = sizeof(char) + sizeof(uint16_t) + 1;

int send_message(int sockfd, MSG* msg){
	memset(buff, 0, sizeof(buff));

	int bytes_sent;
	MSGtobuffer(buff, msg);
	bytes_sent = send(sockfd, buff, msg->length + EXTRA_SIZE, 0);
	if(bytes_sent < 0)
		perror("\nError Send: ");
	return bytes_sent;
}

int receive_message(int sockfd, MSG* msg){
	memset(buff, 0, sizeof(buff));

	int bytes_recv;
	bytes_recv = recv(sockfd, buff, BUFF_SIZE+EXTRA_SIZE, 0);
	if (bytes_recv < 0) {
		perror("\nError: ");
		return -1;
	} else if (bytes_recv == 0) {
		printf("Connection closed.\n");
		return -1;
	}

	buffertoMSG(buff, msg);
	return bytes_recv;
}

void send_file_data(int sockfd, char* filename) {
	struct stat filestatus;
	int status = stat(filename, &filestatus);
	if (status != 0) {
		perror("\nFile Error: ");
		exit(1);
	}

	int bytes_sent, bytes_read;
	MSG msg;
	msg.opcode = 2;	

	// int total_bytes_sent = 0;
	FILE * send_file = fopen(filename, "rb");

	while((bytes_read = fread(msg.payload, 1, BUFF_SIZE, send_file)) > 0) {
		msg.length = bytes_read;
		msg.payload[bytes_read] = '\0';

		bytes_sent = send_message(sockfd, &msg);
		// if (bytes_sent < 0) {
		// 	// ERROR 
		// 	continue;
		// }
	}

	fclose(send_file);

}

void recv_file_data(int sockfd, char* filename, int opmode, int key) {
	int bytes_received;
	MSG msg;

	// Receive actual file
	FILE * recv_file = fopen(filename, "ab");
	if (!recv_file) {
		perror("\nRecv file error: ");
		return;
	}

	while(1) {
		bytes_received = receive_message(sockfd, &msg);
		if (bytes_received < 0) {
			// ERROR HERE
			break;
		}

		if (msg.length == 0) {	// Final message
			printf("-- EOF EOF EOF --\n");
			break;
		}
		//printf("%s\n" , msg.payload);
		handle_file(msg.payload, opmode, key);
		fwrite(msg.payload, 1, msg.length, recv_file);
	}

	fclose(recv_file);
}

void handle_msg(int sockfd, MSG* msg) {
	int key = atoi(msg->payload);
	int opmode = msg->opcode;
	char filename[] = "file-received/file_server.txt";
	// Receive and encode/decode file
	recv_file_data(sockfd, filename, opmode, key);

	// Send file back
	send_file_data(sockfd, filename);

	// Delete temporary file
	delete_file(filename);
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
        fprintf(stderr, "Usage: %s <Port_Number>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);

	int i, maxi, maxfd, listenfd, connfd, sockfd;
	int nready, client[FD_SETSIZE];
	ssize_t	ret;
	fd_set	readfds, allset;
	//char sendBuff[BUFF_SIZE], rcvBuff[BUFF_SIZE];
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;

	//Step 1: Construct a TCP socket to listen connection request
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /* calls socket() */
		perror("\nError: ");
		return 0;
	}

	//Step 2: Bind address to socket
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(port);
	//servaddr.sin_port        = htons(PORT);

	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))==-1){ /* calls bind() */
		perror("\nError: ");
		return 0;
	} 

	//Step 3: Listen request from client
	if(listen(listenfd, BACKLOG) == -1){  /* calls listen() */
		perror("\nError: ");
		return 0;
	}

	maxfd = listenfd;			/* initialize */
	maxi = -1;				/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;			/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	
	//Step 4: Communicate with clients
	while (1) {
		readfds = allset;		/* structure assignment */
		nready = select(maxfd+1, &readfds, NULL, NULL, NULL);
		if(nready < 0){
			perror("\nError: ");
			return 0;
		}
		
		if (FD_ISSET(listenfd, &readfds)) {	/* new client connection */
			clilen = sizeof(cliaddr);
			if((connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen)) < 0)
				perror("\nError: ");
			else{
				printf("You got a connection from %s\n", inet_ntoa(cliaddr.sin_addr)); /* prints client's IP */
				for (i = 0; i < FD_SETSIZE; i++)
					if (client[i] < 0) {
						client[i] = connfd;	/* save descriptor */
						break;
					}
				if (i == FD_SETSIZE){
					printf("\nToo many clients");
					close(connfd);
				}

				FD_SET(connfd, &allset);	/* add new descriptor to set */
				if (connfd > maxfd)
					maxfd = connfd;		/* for select */
				if (i > maxi)
					maxi = i;		/* max index in client[] array */

				if (--nready <= 0)
					continue;		/* no more readable descriptors */
			}
		}

		MSG msg;

		for (i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i]) < 0)
				continue;
			if (FD_ISSET(sockfd, &readfds)) {
				ret = receive_message(sockfd, &msg);
				if (ret <= 0){
					FD_CLR(sockfd, &allset);
					close(sockfd);
					client[i] = -1;
				}
				else {
					handle_msg(sockfd,&msg);
					if (ret <= 0){
						FD_CLR(sockfd, &allset);
						close(sockfd);
						client[i] = -1;
					}
				}

				if (--nready <= 0)
					break;		/* no more readable descriptors */
			}
		}
	}
	
	return 0;
}


