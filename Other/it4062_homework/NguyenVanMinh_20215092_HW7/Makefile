CC = gcc
CFLAGS = -Wall -pthread
PROGRAM = server client
SERVER_DIR = TCP_Server
CLIENT_DIR = TCP_Client
DEPS = tcp_socket.h utility.h session.h
OBJ = include/utility.o include/tcp_socket.o include/session.o

#-----------
all: clean $(PROGRAM)

#-----------
server: $(SERVER_DIR)/server.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

client: $(CLIENT_DIR)/client.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c $(DEPS)
	$(CC) -c $< -o $@ $(CFLAGS)

#-----------
clean:
	rm -f *.o $(SERVER_DIR)/*.o $(CLIENT_DIR)/*.o include/*.o $(PROGRAM)
