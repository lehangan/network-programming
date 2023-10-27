#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#define MAXLINE 4096 /*max text line length*/
// #define SERV_PORT 3000 /*port*/
#define LISTENQ 10 /*maximum number of client connections */

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

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        perror("Usage: <Port_Number> of the server");
        exit(1);
    }

    int SERV_PORT = atoi(argv[1]);

    int listenfd, connfd, n;
    socklen_t clilen;
    char buf[MAXLINE];
    struct sockaddr_in cliaddr, servaddr;

    // creation of the socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if ( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Problem in creating the socket");
        exit(2);
    }
    
    // preparation of the socket address
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)))
    {
        perror("Error: ");
        return 0;
    }

    if(listen(listenfd, LISTENQ)){
        perror("Error! Cannot listen.");
        return 0;
    }

    printf("%s\n", "Server running...waiting for connections.");

    for (;;)
    {

        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        printf("%s\n", "Received request...");

        while ((n = recv(connfd, buf, MAXLINE, 0)) > 0)
        {
            printf("%s", "String received from and resent to the client:");
            buf[n - 1] = '\0';
            printf("%s\n", buf);
            // puts(buf);
            if (!strcmp(buf, ""))
                close(connfd);
            processString(buf);
            send(connfd, result, n, 0);
        }

        if (n < 0)
        {
            perror("Read error");
            exit(1);
        }
        close(connfd);
    }
    // close listening socket
    close(listenfd);
}
