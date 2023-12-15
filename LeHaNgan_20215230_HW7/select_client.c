#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>

// #include "file_handle.h"

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5500
#define BUFF_SIZE 1024

typedef struct MSG{
	int opcode;
	int length;
	char payload[BUFF_SIZE];
}MSG;

void MSGtobuffer(char* buffer, MSG* msg){
	// opcode: 1 bytes
	memcpy(buffer, &msg->opcode, sizeof(char));
	buffer += sizeof(char); // Move buffer pointer

	// length: 2 bytes
	uint16_t netw_length = htons(msg->length);	// Host long to Network long
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
	bytes_sent = send(sockfd, buff, msg->length+EXTRA_SIZE, 0);
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

		printf("%s\n" , msg.payload);
		bytes_sent = send_message(sockfd, &msg);
		// if (bytes_sent < 0) {
		// 	// ERROR 
		// 	continue;
		// }
	}

	fclose(send_file);

	msg.length = 0;
	memset(msg.payload, 0, BUFF_SIZE);
	bytes_sent = send_message(sockfd, &msg);	// Final message
	if (bytes_sent < 0) {
		// ERROR HERE
		// continue;
	}

}

void recv_file_data(int sockfd, char* filename, int opmode, int key) {
	int bytes_received;
	int bytes_sent;
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

		fwrite(msg.payload, 1, msg.length, recv_file);
	}

	msg.length = 0;
	msg.opcode = 2;
	memset(msg.payload, 0, BUFF_SIZE);
	bytes_sent = send_message(sockfd, &msg);

	fclose(recv_file);
}


int main(int argc, char *argv[]){
	if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP_Addr> <Port_Number>\n", argv[0]);
        exit(1);
    }

	char *server_ip = argv[1];
    int port = atoi(argv[2]);
	
	int client_sock;
	//char buff[BUFF_SIZE + 1];
	struct sockaddr_in server_addr; /* server's address information */
	int msg_len, bytes_sent, bytes_received;
	
	//Step 1: Construct socket
	client_sock = socket(AF_INET,SOCK_STREAM,0);
	
	//Step 2: Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(server_ip);
	
	//Step 3: Request to connect server
	if(connect(client_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0){
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}
		
	//Step 4: Communicate with server			
	
	char filename[1024];

	printf("\nFile name to send: ");
	fgets(filename, BUFF_SIZE, stdin);
	filename[strcspn(filename, "\r\n")] = 0;		// Remove all \r\n

	int opcode, key;
	printf("Send mode (0: Encode, 1: Decode): ");
	scanf(" %d", &opcode);

	printf("Key (int): ");
	scanf(" %d", &key);
	
	MSG msg;
	msg.opcode = opcode;
	sprintf(msg.payload, "%d", key);
	msg.length = strlen(msg.payload) + 1;

	// Client sends the request message of encoding/decoding with the key value to server 
	send_message(client_sock, &msg);

	// Client sends data of the file to server
	send_file_data(client_sock, filename);

	// Received encode or decode file from server
	recv_file_data(client_sock, "received_file.txt", msg.opcode, key);

	//Step 5: Close socket
	close(client_sock);

	return 0;
}
