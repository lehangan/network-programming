#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define FILE_NAME_LEN 30
#define BUFFER_SIZE 10000+200
#define LISTENQ 10

char file_path[200];

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s PortNumber\n", argv[0]);
        return 1;
    }

    int SERV_PORT = atoi(argv[1]);

    int listenfd, connfd, n;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create a socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Problem in creating the socket");
        exit(2);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the server socket
    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Binding failed");
        return 0;
    }

    // Listen for incoming connections
    if (listen(listenfd, LISTENQ) == -1)
    {
        perror("Error! Cannot listen.");
        return 0;
    }

    printf("%s\n", "Server running...waiting for connections.");

    for (;;)
    {
        // // Accept a connection from a client
        // connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addr_len);
        // if (connfd == -1)
        // {
        //     perror("Accepting connection failed");
        //     exit(1);
        // }

        // // Receive the file from the client and save it
        // char buffer[BUFFER_SIZE];
        // FILE *file = fopen("received_file", "wb");
        // int bytes_received;

        // while ((bytes_received = recv(connfd, buffer, BUFFER_SIZE, 0)) > 0)
        // {
        //     fwrite(buffer, 1, bytes_received, file);
        // }

        // printf("File received successfully.\n");

        // // Close the sockets and file
        // fclose(file);
        // close(listenfd);

        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (connfd == -1)
        {
            perror("Accepting connection failed");
            exit(1);
        }
        printf("%s\n", "Received request...");

        char buffer[BUFFER_SIZE];
        while ((n = recv(connfd, buffer, BUFFER_SIZE, 0)) > 0)
        {
            // printf("%s", "String received from and resent to the client:");
            buffer[n - 1] = '\0';
            printf("%s\n", buffer);
            
            char *file_name = strtok(buffer, "\n");
            char *file_data = strtok(NULL, "");
            strcpy(file_path, "/home/lehangan/network-programming/file-received/");
            strcat(file_path, file_name);

            printf("%s\n", file_path);

            FILE *file = fopen(file_path, "wb");
            int bytes_received;

            // while ((bytes_received = recv(connfd, buffer, BUFFER_SIZE, 0)) > 0)
            // {
            //     fwrite(buffer, 1, bytes_received, file);
            // }

            fwrite(file_data, 1, strlen(file_data)+1, file);
            printf("%s" , file_data);
            printf("File received successfully.\n");

            // Close the sockets and file
            fclose(file);

            if (!strcmp(buffer, ""))
                close(connfd);
            send(connfd, buffer, n, 0);
        }

        if (n < 0)
        {
            perror("Read error");
            exit(1);
        }
        close(connfd);
    }
    return 0;
}
