#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>

#define PORT 5500
#define BACKLOG 20
#define BUFF_SIZE 1024
#define SERVER_ADDR "127.0.0.1"

// Remember to use -pthread when compiling this server's source code
void *connection_handler(void *client_socket)
{
    int sockfd = *(int *)client_socket;
    char buff[BUFF_SIZE];
	int bytes_sent, bytes_received;
	while (1)
	{
		bytes_received = recv(sockfd, buff, sizeof(buff), 0); // blocking
		if (bytes_received < 0)
			perror("\nError: ");
		else if (bytes_received == 0)
		{
			printf("Connection closed.");
			break;
		}

		buff[bytes_received] = '\0';
		for (int i = 0; buff[i]!= '\0' ; i++)
		{
			buff[i] = toupper(buff[i]);
		}

		if (buff[0] == 'q' || buff[0] == 'Q')
		{
			break;
		}

		bytes_sent = send(sockfd, buff, bytes_received, 0); /* echo to the client */

		if (bytes_sent < 0)
			perror("\nError: ");
	}
	close(sockfd);
    return 0;
}

int main()
{
    // char server_message[] = "Hello from Server!!\n";
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == -1)
    {
        perror("Socket initialisation failed");
        exit(EXIT_FAILURE);
    }
    else
        printf("Server socket created successfully\n");

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR); //INADDR_ANY;

    // bind the socket to the specified IP addr and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");

    if (listen(server_socket, 3) != 0)
    {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");

    int no_threads = 0;
    pthread_t threads[3];
    while (no_threads < 3)
    {
        printf("Listening...\n");
        int client_socket = accept(server_socket, NULL, NULL);
        puts("Connection accepted");
        if (pthread_create(&threads[no_threads], NULL, connection_handler, &client_socket) < 0)
        {
            perror("Could not create thread");
            return 1;
        }
        if (client_socket < 0)
        {
            printf("server acccept failed...\n");
            exit(0);
        }
        else
            printf("Server acccept the client...\n");
        puts("Handler assigned");
        no_threads++;
    }
    int k = 0;
    for (k = 0; k < 3; k++)
    {
        pthread_join(threads[k], NULL);
    }

    // int send_status;
    // send_status=send(client_socket, server_message, sizeof(server_message), 0);
    close(server_socket);

    return 0;
}
