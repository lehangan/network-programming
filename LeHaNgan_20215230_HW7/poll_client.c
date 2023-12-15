#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <stdlib.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5550
#define BUFF_SIZE 1024

char username[100];
char password[100];

#define STATUS_LOCKED 0
#define STATUS_WRONG_PASSWORD 1
#define STATUS_LOGGED_IN 2
#define STATUS_NO_ACCOUNT 3

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
    char buff[BUFF_SIZE + 1];
    struct sockaddr_in server_addr; /* server's address information */
    int msg_len, bytes_sent, bytes_received;

    // Step 1: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    // Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    // Step 3: Request to connect server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("\nError!Can not connect to sever! Client exit imediately! ");
        return 0;
    }

    // Step 4: Communicate with server
    //  + Each client terminal can only login a single account

    while (1)
    {

        printf("\n[SIGN IN]");
        printf("\nUsername: ");
        scanf(" %s", username);
        printf("Password: ");
        scanf(" %s", password);

        memset(buff, '\0', BUFF_SIZE); // Clear memory
        sprintf(buff, "%s %s", username, password);

        bytes_sent = send(client_sock, buff, strlen(buff), 0);
        if (bytes_sent < 0)
            perror("\nError: ");

        // receive echo reply
        bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
        if (bytes_received < 0)
            perror("\nError: ");
        else if (bytes_received == 0)
        {
            printf("Echo: Connection closed.\n");
            break;
        }
        buff[bytes_received] = '\0';

        int account_status = -1;
        sscanf(buff, " %d", &account_status);

        switch (account_status)
        {
        case STATUS_WRONG_PASSWORD:
            printf("\nPassword is incorrect.");
            break;
        case STATUS_NO_ACCOUNT:
            printf("\nUsername doesn't exist.");
            break;

        case STATUS_LOGGED_IN:
            printf("\nLogged in. Congratulations!\n");
            return;
            break;
        case STATUS_LOCKED:
            printf("\nAccount locked. (5 or more failed login attempts)\n");
            return;
            break;
        default:
            perror("\nSOMETHING WRONG HAPPENED? TERMINATING.\n");
            break;
        }
    }

    // Step 4: Close socket
    close(client_sock);
    return 0;
}
