#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <netdb.h>
#include <arpa/inet.h>

#define QSIZE 8    /* size of input queue */
#define MAXDG 4096 /* max datagram size */
#define SERV_PORT 5500

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
                // printf("Not found information\n");
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
            // printf("Not found information\n");
            strcpy(result, "Not found information\n");
            return;
        }
        else
        {
            // printf("Official name: %s\n", hostInfo->h_name);
            strcpy(result, "Official name: ");
            strcat(result, hostInfo->h_name);
            strcat(result, "\n");
            if (hostInfo->h_aliases[0] != NULL)
            {
                // printf("Alias name:\n");
                strcat(result, "Alias name:\n");
                for (char **alias = hostInfo->h_aliases; *alias != NULL; alias++)
                {
                    // printf("%s\n", *alias);
                    strcat(result, *alias);
                    strcat(result, "\n");
                }
            }
        }
    }
}

typedef struct
{
    void *dg_data;          /* ptr to actual datagram */
    size_t dg_len;          /* length of datagram */
    struct sockaddr *dg_sa; /* ptr to sockaddr{} w/client's address */
    socklen_t dg_salen;     /* length of sockaddr{} */
} DG;
static DG dg[QSIZE]; /* queue of datagrams to process */

static int iget;         /* next one for main loop to process */
static int iput;         /* next one for signal handler to read into */
static int nqueue;       /* # on queue for main loop to process */
static socklen_t clilen; /* max length of sockaddr{} */
static int sockfd;

static void sig_io(int);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <Port_Number>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);

    int i;
    const int on = 1;
    sigset_t zeromask, newmask, oldmask;

    struct sockaddr_in servaddr, cliaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    { /* calls socket() */
        perror("socket() error\n");
        return 0;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    { /* calls bind() */
        perror("bind() error\n");
        return 0;
    }

    clilen = sizeof(cliaddr);

    for (i = 0; i < QSIZE; i++)
    { /* init queue of buffers */
        dg[i].dg_data = malloc(MAXDG);
        dg[i].dg_sa = malloc(clilen);
        dg[i].dg_salen = clilen;
    }
    iget = iput = nqueue = 0;

    /* Signal handlers are established for SIGIO. The socket owner is
     * set using fcntl and the signal-driven and non-blocking I/O flags are set using ioctl
     */
    signal(SIGIO, sig_io);
    fcntl(sockfd, F_SETOWN, getpid());
    ioctl(sockfd, FIOASYNC, &on);
    ioctl(sockfd, FIONBIO, &on);

    /* Three signal sets are initialized: zeromask (which never changes),
     * oldmask (which contains the old signal mask when we block SIGIO), and newmask.
     */
    sigemptyset(&zeromask);
    sigemptyset(&oldmask);
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGIO); /* signal we want to block */

    /* Stores the current signal mask of the process in oldmask and then
     * logically ORs newmask into the current signal mask. This blocks SIGIO
     * and returns the current signal mask. We need SIGIO blocked when we test
     * nqueue at the top of the loop
     */
    sigprocmask(SIG_BLOCK, &newmask, &oldmask);

    for (;;)
    {
        while (nqueue == 0)
            sigsuspend(&zeromask); /* wait for datagram to process */

        /* unblock SIGIO by calling sigprocmask to set the signal mask of
         * the process to the value that was saved earlier (oldmask).
         * The reply is then sent by sendto.
         */
        sigprocmask(SIG_SETMASK, &oldmask, NULL);

        sendto(sockfd, dg[iget].dg_data, dg[iget].dg_len, 0,
               dg[iget].dg_sa, dg[iget].dg_salen);

        if (++iget >= QSIZE)
            iget = 0;

        /* SIGIO is blocked and the value of nqueue is decremented.
         * We must block the signal while modifying this variable since
         * it is shared between the main loop and the signal handler.
         */
        sigprocmask(SIG_BLOCK, &newmask, &oldmask);
        nqueue--;
    }
}

static void sig_io(int signo)
{
    ssize_t len;
    DG *ptr;

    for (;;)
    {
        if (nqueue >= QSIZE)
        {
            perror("receive overflow");
            break;
        }

        ptr = &dg[iput];
        ptr->dg_salen = clilen;
        len = recvfrom(sockfd, ptr->dg_data, MAXDG, 0,
                       ptr->dg_sa, &ptr->dg_salen);
        if (len < 0)
        {
            if (errno == EWOULDBLOCK)
                break; /* all done; no more queued to read */
            else
            {
                perror("recvfrom error");
                break;
            }
        }
        
        // Reset the result array before processing new data
        memset(result, 0, sizeof(result));
        solve((char *)ptr->dg_data);
        printf("\ninfo: %s\n", result);
        // Send back the result to the client
        sendto(sockfd, result, strlen(result), 0,
               ptr->dg_sa, ptr->dg_salen);

        ptr->dg_len = len;

        nqueue++;
        if (++iput >= QSIZE)
            iput = 0;
    }
}
