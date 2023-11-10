CC = gcc
CFLAGS = -Wall

SERVER_SRC1 = TcpServeForkEx1.c
SERVER1 = server1

CLI_SRC1 = TcpClientForkEx1.c
CLIENT1 = client1

SERVER_SRC2 =TcpServerForkEx2.c
SERVER2 = server2

CLI_SRC2 = TcpClientForkEx2.c
CLIENT2 = client2

all: $(SERVER1) $(CLIENT1) $(SERVER2) $(CLIENT2)

# Exercise 1
$(SERVER1): $(SERVER_SRC1)
	$(CC) $(CFLAGS) $(SERVER_SRC1) -o $(SERVER1)

$(CLIENT1): $(CLI_SRC1)
	$(CC) $(CFLAGS) $(CLI_SRC1) -o $(CLIENT1)

#Exercise 2
$(SERVER2): $(SERVER_SRC2)
	$(CC) $(CFLAGS) $(SERVER_SRC2) -o $(SERVER2)

$(CLIENT2): $(CLI_SRC2)
	$(CC) $(CFLAGS) $(CLI_SRC2) -o $(CLIENT2)

clean:
	rm -f $(SERVER1) $(CLIENT1) $(SERVER2) $(CLIENT2) 
