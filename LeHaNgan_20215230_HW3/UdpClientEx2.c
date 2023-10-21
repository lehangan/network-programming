#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXLINE 250


int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s IPAddress and PortNumber\n", argv[0]);
        return 1;
    }

    char *IP_ADDRESS = argv[1];
    int SERV_PORT = atoi(argv[2]);

    int sockfd, n;
    struct sockaddr_in servaddr, from_socket;
    socklen_t addrlen = sizeof(from_socket);
    char sendline[MAXLINE], recvline[MAXLINE + 1];

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Error: ");
        return 0;
    }

    while (fgets(sendline, MAXLINE, stdin) != NULL)
    {
        if( strlen(sendline) > 0 && sendline[strlen(sendline)-1] == '\n')
        {
            sendline[strlen(sendline) - 1] = '\0';
        }
        // printf("To Server: %s", sendline);
        sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
        n = recvfrom(sockfd, recvline, MAXLINE, 0, (struct sockaddr *)&from_socket, &addrlen);
        recvline[n] = '\0'; // null terminate
        printf("%s\n" , recvline);
    }
    close(sockfd);

}
