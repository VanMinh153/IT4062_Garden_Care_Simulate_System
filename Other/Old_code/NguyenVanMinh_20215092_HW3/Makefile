CC = gcc
CPP = g++
CFLAGS = -Wall
NET = server server? client client?
NET1 = server client
HEADER = ./Library/utility.h ./Library/resolver.h
LIBSOURCE = utility resolver
#-----------
all: $(NET1)

#-----------
test: udp_test.c
	gcc -pthread udp_test.c -o test

#-----------
server: ./UDP_Server/server.c ./Library/utility.c ./Library/resolver.c $(HEADER)
	$(CC) -o $@ $^ $(CFLAGS)
client: ./UDP_Client/client.c ./Library/utility.c $(HEADER)
	$(CC) -o $@ $^ $(CFLAGS)

#-----------
utility: ./Library/utility.c
	$(CC) -o $@ $^ $(CFLAGS)
resolver: ./Library/resolver.c
	$(CC) -o $@ $^ $(CFLAGS)

#-----------
clean:
	rm -f *.o *.exe* $(NET) $(LIBSOURCE)
