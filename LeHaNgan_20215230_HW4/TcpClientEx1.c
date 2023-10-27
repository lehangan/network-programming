#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#define MAXLINE 4096 /*max text line length*/
// #define SERV_PORT 3000 /*port*/

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
    struct sockaddr_in servaddr;
    char sendline[MAXLINE], recvline[MAXLINE];

    // basic check of the arguments
    // additional checks can be inserted

    // Create a socket for the client
    // If sockfd<0 there was an error in the creation of the socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Problem in creating the socket");
        exit(2);
    }

    // Creation of the socket
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    servaddr.sin_port = htons(SERV_PORT); // convert to big-endian order

    // Connection of the client to the socket
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Problem in connecting to the server");
        exit(3);
    }

    while (fgets(sendline, MAXLINE, stdin) != NULL)
    {

        send(sockfd, sendline, strlen(sendline), 0);

        int ret = recv(sockfd, recvline, MAXLINE, 0);
        if (ret < 0)
        {
            perror("Error: ");
            return 0;
        }
        recvline[ret] = '\0';
        puts(recvline);
    }

    exit(0);
}
