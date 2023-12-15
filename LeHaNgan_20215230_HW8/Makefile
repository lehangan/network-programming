CC = gcc
CFLAGS = -Wall 

SERVER_SRC1 = HW8_Ex1_Server.c
SERVER1 = server1

CLI_SRC1 = HW8_Ex1_Client.c
CLIENT1 = client1

SERVER_SRC2 = HW8_Ex2_Server.c
SERVER2 = server2

CLI_SRC2 = HW8_Ex2_Client.c
CLIENT2 = client2

all: $(SERVER1) $(CLIENT1) $(SERVER2) $(CLIENT2)

# all: $(SERVER1) $(CLIENT1) 

# all: $(SERVER2) $(CLIENT2) 

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
# clean:
# 	rm -f $(SERVER1) $(CLIENT1) 

# clean:
# 	rm -f $(SERVER2) $(CLIENT2) 