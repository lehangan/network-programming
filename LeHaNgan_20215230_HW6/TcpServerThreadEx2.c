#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define MAX_CLIENTS 10
#define MAX_ATTEMPTS 5

struct Account
{
    char accountID[50];
    char password[50];
    int status; // 0 for locked, 1 for unlocked
    int loginAttempts;
};

struct Account accounts[10];
int accCount = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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
	while ((c = fgetc(file)) != EOF) {
		if (c == '\n') {
			lines++;
		}
	}

    // Reset file position to the beginning
	fseek(file, 0, SEEK_SET);

    // Read user accounts into the array
	int info_read = 0;
	for (int i = 0; i < lines; i++) {
		info_read = fscanf(file, "%s %s %d", accounts[i].accountID, accounts[i].password, &accounts[i].status);
		if (info_read != 3) {
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

void *connection_handler(void *client_socket)
{
    int socket = *(int *)client_socket;
    int read_len;
    char client_acc[100];
    char client_pass[100];

    // Lock mutex for thread safety
    pthread_mutex_lock(&mutex);

    read_len = recv(socket, client_acc, sizeof(client_acc), 0);
    client_acc[read_len] = '\0';
    read_len = recv(socket, client_pass, sizeof(client_pass), 0);
    client_pass[read_len] = '\0';

    // Unlock mutex after receiving userID, password
    pthread_mutex_unlock(&mutex);

    int accIndex = findAccount(client_acc, client_pass);

    if (accIndex == -1)
    {
        int index = findAccountIndex(client_acc);

        if (index != -1)
        {
            if (accounts[index].status == 0)
            {
                char mess_wrong[] = "Wrong password & Account locked";
                send(socket, mess_wrong, strlen(mess_wrong), 0);
            }
            else if (accounts[index].loginAttempts < MAX_ATTEMPTS )
            {
                accounts[index].loginAttempts += 1;
                char pass_wrong[] = "Wrong password";
                send(socket, pass_wrong, strlen(pass_wrong), 0);
            }
            else
            {
                accounts[index].status = 0;
                char acc_lock[] = "Account locked";
                send(socket, acc_lock, strlen(acc_lock), 0);
            }
        }
        else
        {
            char msg[] = "Invalid User";
            send(socket, msg, strlen(msg), 0);
        }
    }
    else
    {
        if (accounts[accIndex].status == 0)
        {
            char acc_lock[] = "Account locked";
            send(socket, acc_lock, strlen(acc_lock), 0);
        }
        else
        {
            char acc_unlock[] = "Account unlocked";
            send(socket, acc_unlock, strlen(acc_unlock), 0);
        }
    }
    pthread_mutex_lock(&mutex); // Lock mutex for thread safety
    saveAccounts();
    pthread_mutex_unlock(&mutex); // Unlock mutex after saving user accounts

    free(client_socket); // Free the allocated memory
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_len = sizeof(client_address);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address struct
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) != 0)
    {
        perror("Socket bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) != 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port);

    loadAccounts();

    // pthread_t threads[MAX_CLIENTS];

    while (1)
    {
        // Accept connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len);
        if (client_socket < 0)
        {
            perror("Server accept failed");
            exit(EXIT_FAILURE);
        }

        pthread_t thread;
        // Allocate memory for the client socket
        int *socket_ptr = (int *)malloc(sizeof(int));
        if (socket_ptr == NULL)
        {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        *socket_ptr = client_socket;

        if (pthread_create(&thread, NULL, connection_handler, socket_ptr) != 0)
        {
            perror("Could not create thread");
            free(socket_ptr); // Free the allocated memory
            exit(EXIT_FAILURE);
        }
    }

    // Close the server socket (unreachable in this example)
    close(server_socket);

    return 0;
}
