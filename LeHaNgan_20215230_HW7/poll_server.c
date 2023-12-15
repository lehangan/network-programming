#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <poll.h>
#include <limits.h>

#define POLLRDNORM 0x0040 /* Normal data may be read */
#define LISTENQ 5
#define BUFF_SIZE 1024

#define STATUS_LOCKED 0
#define STATUS_WRONG_PASSWORD 1
#define STATUS_LOGGED_IN 2
#define STATUS_NO_ACCOUNT 3

struct Account
{
    char accountID[50];
    char password[50];
    int status; // 0 for locked, 1 for unlocked
    int loginAttempts;
};

struct Account accounts[10];
int accCount = 0;

void loadAccounts()
{
    FILE *file = fopen("account.txt", "r");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Count the number of lines in the file
    int lines = 1;
    char c;
    while ((c = fgetc(file)) != EOF)
    {
        if (c == '\n')
        {
            lines++;
        }
    }

    // Reset file position to the beginning
    fseek(file, 0, SEEK_SET);

    // Read user accounts into the array
    int info_read = 0;
    for (int i = 0; i < lines; i++)
    {
        info_read = fscanf(file, "%s %s %d", accounts[i].accountID, accounts[i].password, &accounts[i].status);
        if (info_read != 3)
        {
            printf("Err: Error reading line %d\n", i + 1);
            continue;
        }
        // Add to global
        accCount++;
    }

    fclose(file);
}

void saveAccounts()
{
    FILE *file = fopen("account.txt", "w");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < accCount; i++)
    {
        fprintf(file, "%s %s %d\n", accounts[i].accountID, accounts[i].password, accounts[i].status);
    }

    fclose(file);
}

void accountAddLoginCount(int acc)
{ // i = accountIndex
    accounts[acc].loginAttempts++;
}

int accountCheckLocked(int acc)
{
    int over5 = accounts[acc].status;
    return over5;
}

int findAccount(const char *acc, const char *password)
{
    for (int i = 0; i < accCount; i++)
    {
        if (strcmp(accounts[i].accountID, acc) == 0 && strcmp(accounts[i].password, password) == 0)
        {
            return i;
        }
    }
    return -1;
}

int findAccountIndex(char *acc)
{
    for (int i = 0; i < accCount; i++)
    {
        if (strcmp(accounts[i].accountID, acc) == 0)
        {
            return i;
        }
    }
    return -1; // not found
}

void accountResetLoginCount(int acc)
{
    accounts[acc].loginAttempts = 0;
}

void handle_server(int sockfd, int *pollclientfd)
{
    int accIndex = -1;
    char buff[BUFF_SIZE + 1];
    int bytes_sent, bytes_received;
    char username[100];
    char password[100];

    bytes_received = recv(sockfd, buff, BUFF_SIZE, 0); // Get username
    if (bytes_received < 0)
    {
        perror("\nError: ");
        close(sockfd);
        *pollclientfd = -1;
    }
    else if (bytes_received == 0)
    {
        printf("Echo: Connection closed.\n");
        close(sockfd);
        *pollclientfd = -1;
        return;
    }
    buff[bytes_received] = '\0';

    /**********************/
    /*** Your code here ***/
    sscanf(buff, "%s %s", username, password);
    int status = -1;

    accIndex = findAccountIndex(username);

    if (accIndex == -1)
    {
        // Account doesn't exist
        status = STATUS_NO_ACCOUNT;
    }
    else
    {
        // Account exist
        if (findAccount(username, password) == 1)
        {
            status = STATUS_LOGGED_IN;
            accountResetLoginCount(accIndex);
            saveAccounts();
        }
        else
        {
            // Wrong password
            status = STATUS_WRONG_PASSWORD;
            accountAddLoginCount(accIndex);
            saveAccounts();
        }

        // Over 5 times
        if (accountCheckLocked(accIndex) == 1)
        {
            status = STATUS_LOCKED;
        }
    }

    memset(buff, '\0', BUFF_SIZE); // Clear memory
    sprintf(buff, "%d", status);
    /**********************/
    /**********************/

    bytes_sent = send(sockfd, buff, bytes_received, 0); /* echo to the client */
    if (bytes_sent < 0)
    {
        perror("\nError: ");
    }
    // }
}

void usage(char *program)
{
    fprintf(stderr, "usage: %s port\n", program);
}

int main(int argc, char *argv[])
{
    loadAccounts();
    int nready, i, maxi, listenfd, connfd, sockfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    char buf[BUFF_SIZE];
    const int OPEN_MAX = sysconf(_SC_OPEN_MAX); // maximum number of opened files
    struct pollfd clients[OPEN_MAX];
    ssize_t n;
    int INFTIM = -1;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <Port_Number>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);

    if (port <= 0 || port > 65535)
    {
        fprintf(stderr, "Invalid port number %d\n", port);
        return EXIT_FAILURE;
    }

    // Create listen socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "Error: socket\n");
        return EXIT_FAILURE;
    }
    else
    {
        printf("Create listen socket %d\n", listenfd);
    }

    // Initialize server socket address
    // memset(&servaddr, 0, sizeof(servaddr));
    // servaddr.sin_family = AF_INET;
    // servaddr.sin_addr.s_addr = INADDR_ANY;
    // servaddr.sin_port = htons(port);

    bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(port);
    
    // Bind socket to an address
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        fprintf(stderr, "Error: bind\n");
        return EXIT_FAILURE;
    }

    // Listen
    if (listen(listenfd, LISTENQ) < 0)
    {
        fprintf(stderr, "Error: listen\n");
        return EXIT_FAILURE;
    }

    clients[0].fd = listenfd;
    clients[0].events = POLLRDNORM;

    for (i = 1; i < OPEN_MAX; i++)
    {
        clients[i].fd = -1; // -1 indicates available entry
    }
    maxi = 0; // max index into clients[] array

    while (1)
    {
        nready = poll(clients, maxi + 1, INFTIM);

        if (nready <= 0)
        {
            continue;
        }

        // Check new connection
        if (clients[0].revents & POLLRDNORM)
        {
            clilen = sizeof(cliaddr);
            if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0)
            {
                fprintf(stderr, "Error: accept\n");
                return EXIT_FAILURE;
            }

            printf("Accept socket %d (%s : %hu)\n",
                   connfd,
                   inet_ntoa(cliaddr.sin_addr),
                   ntohs(cliaddr.sin_port));

            // Save client socket into clients array
            for (i = 0; i < OPEN_MAX; i++)
            {
                if (clients[i].fd < 0)
                {
                    clients[i].fd = connfd;
                    break;
                }
            }

            // No enough space in clients array
            if (i == OPEN_MAX)
            {
                fprintf(stderr, "Error: too many clients\n");
                close(connfd);
            }

            clients[i].events = POLLRDNORM;

            if (i > maxi)
            {
                maxi = i;
            }

            // No more readable file descriptors
            if (--nready <= 0)
            {
                continue;
            }
        }

        // Check all clients to read data
        for (i = 1; i <= maxi; i++)
        {
            if ((sockfd = clients[i].fd) < 0)
            {
                continue;
            }

            // If the client is readable or errors occur
            if (clients[i].revents & (POLLRDNORM | POLLERR))
            {
                handle_server(sockfd, &clients[i].fd);

                // No more readable file descriptors
                if (--nready <= 0)
                {
                    break;
                }
            }
        }
    }

    close(listenfd);
    return EXIT_SUCCESS;
}