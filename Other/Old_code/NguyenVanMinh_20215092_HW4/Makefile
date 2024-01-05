CC = gcc
CPP = g++
CFLAGS = -Wall
NET_ALL = *server *client
NET = server client
HEADER = ./Library/utility.h
LIBSOURCE = utility
#-----------
all: clean $(NET)

#-----------
server: ./TCP_Server/server.c ./Library/utility.c
	$(CC) -o $@ $^ $(CFLAGS)
client: ./TCP_Client/client.c ./Library/utility.c
	$(CC) -o $@ $^ $(CFLAGS)

#-----------
clean:
	rm -f *.o $(NET)
