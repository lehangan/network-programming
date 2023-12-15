/*UDP Echo Client*/
#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define SERV_PORT 5500
#define SERV_IP "127.0.0.1"
#define BUFF_SIZE 4096

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s <IP_Addr> <Port_Number>\n", argv[0]);
		exit(1);
	}

	char *server_ip = argv[1];
	int port = atoi(argv[2]);

	int client_sock;
	char buff[BUFF_SIZE];
	struct sockaddr_in server_addr;
	int bytes_sent, bytes_received, sin_size;

	// Step 1: Construct a UDP socket
	if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{ /* calls socket() */
		perror("\nError: ");
		exit(0);
	}

	// Step 2: Define the address of the server
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(server_ip);

	// Step 3: Communicate with server
	sin_size = sizeof(struct sockaddr);
	while (1)
	{
		printf("\nEnter an IP address or domain name (blank to exit): ");
		memset(buff, '\0', (strlen(buff) + 1));
		if (fgets(buff, BUFF_SIZE, stdin) == NULL)
		{
			break;
		}
		
		size_t len = strlen(buff);
        if (len > 0 && buff[len - 1] == '\n')
        {
            buff[len - 1] = '\0';
        }
        if (strcmp(buff, "") == 0)
        {
            printf("Sending a stop signal to the server.\n");
            sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
            break;
        }

		bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *)&server_addr, sin_size);
		if (bytes_sent < 0)
		{
			perror("Error: ");
			close(client_sock);
			return 0;
		}
			
		bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&server_addr, &sin_size);
		if (bytes_received < 0)
		{
			perror("Error: ");
			close(client_sock);
			return 0;
		}

		buff[bytes_received] = '\0';
		printf("Reply from server: %s", buff);
	}

	close(client_sock);
	return 0;
}
