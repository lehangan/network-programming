#include <bits/stdc++.h>
#include <regex>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/unistd.h>

using namespace std;

bool isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    // The value 0 if the address is invalid
    // The value non-zero if the address is valid
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result;
}

bool isValidDomain(string str)
{

    // Regex to check valid domain name.
    const regex pattern("^(?!-)[A-Za-z0-9-]+([\\-\\.]{1}[a-z0-9]+)*\\.[A-Za-z]{2,6}$");

    // If the domain name
    // is empty return false
    if (str.empty())
    {
        return false;
    }

    // Return true if the domain name
    // matched the ReGex
    if (regex_match(str, pattern))
    {
        return true;
    }
    else
    {
        return false;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s IP_address\n", argv[0]);
        return 1;
    }

    char *input = argv[1];
    if (!isValidIpAddress(input))
    {
        if (!isValidDomain(input))
        {
            printf("Invalid Address\n");
            return 0;
        }
        struct hostent *host_entry;
        struct in_addr **addr_list;

        // Retrieve IP addresses
        host_entry = gethostbyname(input);
        if (host_entry == NULL)
        {
            printf("Not found information\n");
            return 0;
        }
        else
        {
            addr_list = (struct in_addr **)host_entry->h_addr_list;
            if (addr_list[0] != NULL)
            {
                printf("Official IP: %s\n", inet_ntoa(*addr_list[0]));
                for (int i = 1; addr_list[i] != NULL; i++)
                {
                    printf("Alias IP:\n");
                    printf("%s\n", inet_ntoa(*addr_list[i]));
                }
            }
            else
                printf("Not found information\n");
        }
    }
    else if (isValidIpAddress(input))
    {
        struct in_addr ipv4_addr;
        inet_aton(input, &ipv4_addr);
        struct hostent *hostInfo = gethostbyaddr(&ipv4_addr, sizeof(struct in_addr), AF_INET);
        if (hostInfo == NULL)
        {
            printf("Not found information\n");
            return 0;
        }
        else
        {
            printf("Official name: %s\n", hostInfo->h_name);
            if (hostInfo->h_aliases[0] != NULL)
            {
                printf("Alias name:\n");
                for (char **alias = hostInfo->h_aliases; *alias != NULL; alias++)
                {
                    printf("%s\n", *alias);
                }
            }
        }
    }

    return 0;
}
