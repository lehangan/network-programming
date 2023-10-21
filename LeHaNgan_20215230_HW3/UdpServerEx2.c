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
#include <stdbool.h>

#define MAXLINE 255
bool isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    // The value 0 if the address is invalid
    // The value non-zero if the address is valid
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result;
}

bool isValidDomain(const char *str)
{
    if (str == NULL || strlen(str) == 0)
    {
        return false;
    }

    // Check the overall length of the string
    if (strlen(str) > 255)
    {
        return false;
    }

    // Check for valid characters
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (!((str[i] >= 'a' && str[i] <= 'z') ||
              (str[i] >= 'A' && str[i] <= 'Z') ||
              (str[i] >= '0' && str[i] <= '9') ||
              str[i] == '-' || str[i] == '.'))
        {
            return false;
        }
    }

    // Check for consecutive dots or dashes
    for (size_t i = 0; i < strlen(str); i++)
    {
        if ((str[i] == '-' || str[i] == '.') && (str[i + 1] == '-' || str[i + 1] == '.'))
        {
            return false;
        }
    }

    // Check the last part (TLD) for length and character validity
    char *lastPart = strrchr(str, '.');
    if (lastPart)
    {
        lastPart++; // Move past the dot
        if (strlen(lastPart) < 2 || strlen(lastPart) > 6)
        {
            return false;
        }
        for (size_t i = 0; i < strlen(lastPart); i++)
        {
            if (!(lastPart[i] >= 'a' && lastPart[i] <= 'z'))
            {
                return false;
            }
        }
    }
    else
    {
        return false; // No dot found
    }

    return true;
}

char result[200];

void solve(char *input)
{
    if (!isValidIpAddress(input))
    {
        if (!isValidDomain(input))
        {
            // printf("IP Address is invalid\n");
            strcpy(result, "IP Address is invalid\n");
            return;
        }
        struct hostent *host_entry;
        struct in_addr **addr_list;

        // Retrieve IP addresses
        host_entry = gethostbyname(input);
        if (host_entry == NULL)
        {
            // printf("Not found information\n");
            strcpy(result, "Not found information\n");
            return;
        }
        else
        {
            addr_list = (struct in_addr **)host_entry->h_addr_list;
            if (addr_list[0] != NULL)
            {
                // printf("Official IP: %s\n", inet_ntoa(*addr_list[0]));
                strcpy(result, "Official IP: ");
                strcat(result, inet_ntoa(*addr_list[0]));
                strcat(result, "\n");
                for (int i = 1; addr_list[i] != NULL; i++)
                {
                    // printf("Alias IP:\n");
                    // printf("%s\n", inet_ntoa(*addr_list[i]));
                    strcat(result, "Alias IP:\n");
                    strcat(result, inet_ntoa(*addr_list[i]));
                }
            }
            else
            {
                //printf("Not found information\n");
                strcpy(result, "Not found information\n");
                return;
            }
        }
    }
    else if (isValidIpAddress(input))
    {
        struct in_addr ipv4_addr;
        inet_aton(input, &ipv4_addr);
        struct hostent *hostInfo = gethostbyaddr(&ipv4_addr, sizeof(struct in_addr), AF_INET);
        if (hostInfo == NULL)
        {
            //printf("Not found information\n");
            strcpy(result, "Not found information\n");
            return;
        }
        else
        {
            //printf("Official name: %s\n", hostInfo->h_name);
            strcpy(result, "Official name: ");
            strcat(result, hostInfo->h_name);
            strcat(result, "\n");
            if (hostInfo->h_aliases[0] != NULL)
            {
                //printf("Alias name:\n");
                strcat(result, "Alias name:\n");
                for (char **alias = hostInfo->h_aliases; *alias != NULL; alias++)
                {
                    //printf("%s\n", *alias);
                    strcat(result, *alias );
                    strcat(result, "\n");
                }
            }
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
        if(!strcmp(mesg,"")){
            printf("Close socket");
            close(sockfd);
            return 0;
        }
        printf("Recevied: %s\n", mesg);
        solve(mesg);
        sendto(sockfd, result, strlen(result), 0, (struct sockaddr *)&cliaddr, len);
    }

    // close(sockfd);
}
