#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE 255

int check(char *s)
{
    int n = strlen(s);
    for (int i = 0; i < n; i++)
    {
        if (isdigit(s[i]) || isalpha(s[i]))
            continue;
        else
            return 0;
    }
    return 1;
}
char result[100];

void processString(char *s)
{
    if (!check(s))
    {
        strcpy(result, "Error");
        return;
    }
    else
    {
        char a[100];
        char b[100];
        int n1 = 0;
        int n2 = 0;
        int has_alpha = 0;
        int has_digit = 0;
        for (int i = 0; s[i] != '\0'; i++)
        {
            if (isdigit(s[i]))
            {
                a[n1] = s[i];
                n1++;
                has_digit = 1;
            }
            else 
            {
                b[n2] = s[i];
                n2++;
                has_alpha = 1;
            }
        }
        if (has_alpha && has_digit)
        {
            a[n1] = '\0';
            b[n2] = '\0';
            strcpy(result, a);
            strcat(result, "\n");
            strcat(result, b);
        }
        else if (has_alpha && !has_digit)
        {
            b[n2] = '\0';
            strcpy(result, b);
        }
        else 
        {   
            a[n1] = '\0';
            strcpy(result, a);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s PortNumber\n", argv[0]);
        return 1;
    }

    int SERV_PORT = atoi(argv[1]);

    int sockfd, n;
    socklen_t len;
    char mesg[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Error: ");
        return 0;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) >= 0)
    {
        printf("Server is running at port %d\n", SERV_PORT);
    }
    else
    {
        perror("bind failed");
        return 0;
    }

    for (;;)
    {
        len = sizeof(cliaddr);
        n = recvfrom(sockfd, mesg, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
        mesg[n] = '\0';
        printf("Recevied: %s\n" , mesg);

        if (!strcmp(mesg, "***") || !strcmp(mesg, ""))
        {
            printf("Close socket");
            close(sockfd);
            return 0;
        }
        processString(mesg);
        sendto(sockfd, result, strlen(result), 0, (struct sockaddr *)&cliaddr, len);
    }
}
