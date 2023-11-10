#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <ctype.h>
#include <sys/stat.h>

#define PORT 5501
#define BACKLOG 20
#define BUFF_SIZE 1024
#define SERVER_ADDR "127.0.0.1"

/* Handler process signal*/
// void sig_chld(int signo);

// /*
//  * Receive and echo message to client
//  * [IN] sockfd: socket descriptor that connects to client
//  */
// void echo(int sockfd);

void changeFile(char *str)
{
	for (int i = 0; str[i] != '\0'; i++)
		str[i] = toupper(str[i]);
}

void recv_file_from_client(int sockfd, char *filename)
{
	uint32_t filesize_get = 0;
	int bytes_received;

	bytes_received = recv(sockfd, &filesize_get, sizeof(filesize_get), 0);	// Get file size
	if (bytes_received < 0)
		perror("\nError: ");
	else if (bytes_received == 0) {
		printf("Connection closed.\n");
		return;
	}

	int filesize = ntohl(filesize_get); //network to host long 

	// Receive actual file
	char buff[BUFF_SIZE + 1];
	FILE * recv_file = fopen(filename, "wb");

	while(filesize > 0) {
		bytes_received = recv(sockfd, buff, BUFF_SIZE, 0);
		if (bytes_received < 0)
			perror("\nError: ");
		else if (bytes_received == 0) {
			printf("Connection closed.\n");
			break;
		}
		buff[bytes_received] = '\0';

		changeFile(buff);
		fwrite(buff, 1, bytes_received, recv_file);

		filesize -= bytes_received;
		memset(buff, 0, sizeof(buff));
	}

	fclose(recv_file);
}

int send_file_to_client(int sockfd, char *filename)
{
	struct stat filestatus;
	int status;

	status = stat(filename, &filestatus);
	if (status != 0) {
		perror("\nFile Error: ");
		exit(1);
	}

	int bytes_sent, bytes_read;
		
	// Send file size
	uint32_t filesize_to_send = htonl(filestatus.st_size); //host to network long 
	bytes_sent = send(sockfd, &filesize_to_send, sizeof(filesize_to_send), 0);
	if(bytes_sent < 0) {
		perror("\nError: ");
	}

	// Send actual file
	char buff[BUFF_SIZE + 1];
	FILE * send_file = fopen(filename, "rb");

	int total_bytes_sent = 0;

	while((bytes_read = fread(buff, 1, BUFF_SIZE, send_file)) > 0) {
		buff[bytes_read] = '\0';
		bytes_sent = send(sockfd, buff, bytes_read, 0);
		if(bytes_sent < 0) {
			perror("\nError: ");
		}

		total_bytes_sent += bytes_sent;
		memset(buff, 0, sizeof(buff));
	}

	fclose(send_file);

	return total_bytes_sent;
}

void sig_chld(int signo)
{
	pid_t pid;
	int stat;

	/* Wait the child process terminate */
	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("\nChild %d terminated\n", pid);
}

void echo(int sockfd)
{
	char tempfilename[] = "tempfile.txt";

	while (1)
	{
		recv_file_from_client(sockfd, tempfilename);
		send_file_to_client(sockfd, tempfilename);
	}
}

int main()
{

	int listen_sock, conn_sock; /* file descriptors */
	struct sockaddr_in server;	/* server's address information */
	struct sockaddr_in client;	/* client's address information */
	pid_t pid;
	int sin_size;

	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{ /* calls socket() */
		printf("socket() error\n");
		return 0;
	}

	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = inet_addr(SERVER_ADDR); /* INADDR_ANY puts your IP address automatically */

	if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		perror("\nError: ");
		return 0;
	}

	if (listen(listen_sock, BACKLOG) == -1)
	{
		perror("\nError: ");
		return 0;
	}

	/* Establish a signal handler to catch SIGCHLD */
	signal(SIGCHLD, sig_chld);

	while (1)
	{
		sin_size = sizeof(struct sockaddr_in);
		if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1)
		{
			if (errno == EINTR)
				continue;
			else
			{
				perror("\nError: ");
				return 0;
			}
		}

		/* For each client, fork spawns a child, and the child handles the new client */
		pid = fork();

		/* fork() is called in child process */
		if (pid == 0)
		{
			close(listen_sock);
			printf("You got a connection from %s\n", inet_ntoa(client.sin_addr)); /* prints client's IP */
			echo(conn_sock);
			close(conn_sock);
			exit(0);
		}

		/* The parent closes the connected socket since the child handles the new client */
		close(conn_sock);
	}
	close(listen_sock);
	return 0;
}
