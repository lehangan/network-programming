#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <IP_address> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *ip_address = argv[1];
    int port = atoi(argv[2]);
    int client_socket;
    struct sockaddr_in server_address;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address struct
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    if (inet_pton(AF_INET, ip_address, &server_address.sin_addr) <= 0)
    {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    char userID[100];
    char password[100];
    char response[256];

    printf("Enter userID: ");
    scanf("%s", userID);
    
    // Remove the newline character directly
    if (strlen(userID) > 0 && userID[strlen(userID) - 1] == '\n')
    {
        userID[strlen(userID) - 1] = '\0';
    }

    send(client_socket, userID, strlen(userID), 0);

    printf("Enter password: ");
    scanf("%s", password);
    // Remove the newline character directly
    if (strlen(password) > 0 && password[strlen(password) - 1] == '\n')
    {
        password[strlen(password) - 1] = '\0';
    }

    send(client_socket, password, strlen(password), 0);

    // Receive response from server
    recv(client_socket, response, sizeof(response), 0);
    printf("%s\n", response);

    // Close the client socket
    close(client_socket);

    return 0;
}
