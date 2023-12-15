#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define PORT 5500
#define BUFF_SIZE 1024

int main(int argc, const char *argv[])
{
    // create a socket
    int network_socket;
    network_socket = socket(AF_INET, SOCK_STREAM, 0);

    // specify an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int connection_status = connect(network_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    // check for connection_status
    if (connection_status == -1)
    {
        printf("The connection has error\n\n");
    }
    
	char buff[BUFF_SIZE + 1];
	int msg_len, bytes_sent, bytes_received;
    int total_bytes = 0;
	// send message
	while (1)
	{
		printf("\nInsert string to send:");
		memset(buff, '\0', (strlen(buff) + 1));
		fgets(buff, BUFF_SIZE, stdin);
		msg_len = strlen(buff);
		
		bytes_sent = send(network_socket, buff, msg_len, 0);
		if (bytes_sent < 0)
			perror("\nError: ");

		total_bytes += bytes_sent;

		if (buff[0] == 'q' || buff[0] == 'Q')
		{
			break;
		}

		// receive echo reply
		bytes_received = recv(network_socket, buff, BUFF_SIZE, 0);
		if (bytes_received < 0)
			perror("\nError: ");
		else if (bytes_received == 0)
		{
			printf("Connection closed.\n");
			break;
		}

		buff[bytes_received] = '\0';
		printf("Reply from server: %s", buff);
	}
	// Step 4: Close socket
	printf("Total bytes %d\n", total_bytes);
	close(network_socket);

    return 0;
}
