#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5501
#define BUFF_SIZE 1024

void recv_file_from_server(int sockfd, char *filename)
{
	uint32_t filesize_get = 0;
	int bytes_received;

	bytes_received = recv(sockfd, &filesize_get, sizeof(filesize_get), 0); // Get file size
	if (bytes_received < 0)
		perror("\nError: ");
	else if (bytes_received == 0)
	{
		printf("Connection closed.\n");
		return;
	}

	int filesize = ntohl(filesize_get); //network to host long

	// Receive actual file
	char buff[BUFF_SIZE + 1];
	FILE *recv_file = fopen(filename, "wb");

	while (filesize > 0)
	{
		bytes_received = recv(sockfd, buff, BUFF_SIZE, 0);
		if (bytes_received < 0)
			perror("\nError: ");
		else if (bytes_received == 0)
		{
			printf("Connection closed.\n");
			break;
		}
		buff[bytes_received] = '\0';

		fwrite(buff, 1, bytes_received, recv_file);

		filesize -= bytes_received;
		memset(buff, 0, sizeof(buff));
	}

	fclose(recv_file);
}

int send_file_to_server(int sockfd, char *filename)
{
	struct stat filestatus;
	int status;

	status = stat(filename, &filestatus);
	if (status != 0)
	{
		perror("\nFile Error: ");
		exit(1);
	}

	int bytes_sent, bytes_read;

	// Send file size
	uint32_t filesize_to_send = htonl(filestatus.st_size); //host to network long 
	bytes_sent = send(sockfd, &filesize_to_send, sizeof(filesize_to_send), 0);
	if (bytes_sent < 0)
	{
		perror("\nError: ");
	}

	// Send actual file
	char buff[BUFF_SIZE + 1];
	FILE *send_file = fopen(filename, "rb");

	int total_bytes_sent = 0;

	while ((bytes_read = fread(buff, 1, BUFF_SIZE, send_file)) > 0)
	{
		buff[bytes_read] = '\0';
		bytes_sent = send(sockfd, buff, bytes_read, 0);
		if (bytes_sent < 0)
		{
			perror("\nError: ");
		}

		total_bytes_sent += bytes_sent;
		memset(buff, 0, sizeof(buff));
	}

	fclose(send_file);

	return total_bytes_sent;
}

int main()
{
	int client_sock;
	char buff[BUFF_SIZE + 1];
	struct sockaddr_in server_addr; /* server's address information */
	int msg_len, bytes_sent, bytes_received;

	// Step 1: Construct socket
	client_sock = socket(AF_INET, SOCK_STREAM, 0);

	// Step 2: Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// Step 3: Request to connect server
	if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
	{
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}

	// Step 4: Communicate with server
	// send message

	printf("\nInsert path file to send:");
	memset(buff, '\0', (strlen(buff) + 1));
	fgets(buff, BUFF_SIZE, stdin);
	msg_len = strlen(buff);

	buff[strcspn(buff, "\r\n")] = 0;

	printf("%s\n", buff);

	int total_bytes_sent = send_file_to_server(client_sock, buff);
	recv_file_from_server(client_sock, "received.txt");

	printf("\nTotal bytes sent: %d\n", total_bytes_sent);
	return 0;
}
