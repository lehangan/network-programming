#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <limits.h>
// #define FILE_PATH "testTransfer.txt"
#define BUFFER_SIZE 10000 + 200
#define MAXLINE 4096

char *file_name;

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        perror("Usage: TCPClient <IP address> and <Port_Number> of the server");
        exit(1);
    }

    int SERV_PORT = atoi(argv[2]);
    const char *IP_ADDRESS = argv[1];

    int sockfd;
    struct sockaddr_in server_addr;
    char file_path[MAXLINE], recvline[MAXLINE];
    char send_line[MAXLINE];

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Connection failed");
        exit(1);
    }

    while (fgets(file_path, sizeof(file_path), stdin) != NULL)
    {
        if (!strcmp(file_path, ""))
            close(sockfd);
        if (file_path[strlen(file_path) - 1] == '\n')
        {
            file_path[strlen(file_path) - 1] = '\0';
        }
        // char resolved_path[200];
        // realpath(file_path,resolved_path);
        file_name = strrchr(file_path, '/');
        char file_real_name[200];

        if (file_name)
        {
            // Move to the character after the last separator to get the file name
            file_name++;
            strcpy(file_real_name, file_name);
        }
        else
        {
            strcpy(file_real_name, file_path);
        }

        printf("%s\n", file_real_name);

        FILE *file = fopen(file_path, "rb");
        char buffer[BUFFER_SIZE];
        int bytes_read;

        if (file != NULL)
        {
            printf("File exits\n");
            char *file_data;
            size_t file_size;
            // Determine the size of the file
            fseek(file, 0, SEEK_END);
            file_size = ftell(file);
            fseek(file, 0, SEEK_SET);

            file_data = (unsigned char *)malloc(file_size);
            int file_data_len = fread(file_data, 1, file_size, file);
            printf("%s", file_data);

            memset(buffer, '\0', BUFFER_SIZE);
            sprintf(buffer, "%s\n%s", file_real_name, file_data);

            // printf("%s\n" , buffer);
            // while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
            // {
            //     printf("%s" , buffer);
            //     //send(sockfd, buffer, bytes_read, 0);
            // }

            send(sockfd, buffer, BUFFER_SIZE, file_data_len + strlen(file_real_name) + 1);
            printf("Transfer complete.\n");
            // Close the file after checking
            fclose(file);
        }
        else
        {
            printf("Error: File not found\n");
        }

        // Close the socket and file
        // fclose(file);
    }
    close(sockfd);

    return 0;
}
