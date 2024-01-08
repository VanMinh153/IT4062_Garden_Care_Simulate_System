CC = gcc
CFLAGS = -Wall -I./include
PROGRAM = server client
SERVER_DIR = TCP_Server
CLIENT_DIR = TCP_Client
DEPS = tcp_socket.h hw5_utility.h
OBJ = include/hw5_utility.o include/tcp_socket.o

#-----------
all: clean $(PROGRAM)

#-----------
server: $(SERVER_DIR)/server.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

client: $(CLIENT_DIR)/client.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

test: tcp_test.c
	gcc -pthread tcp_test.c -o test

%.o: %.c $(DEPS)
	$(CC) -c $< -o $@ $(CFLAGS)

#-----------
clean:
	rm -f *.o $(SERVER_DIR)/*.o $(CLIENT_DIR)/*.o include/*.o $(PROGRAM)
