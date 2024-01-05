#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <pthread.h>
#include <errno.h>
#include <poll.h>
#include "../include/utility.h"
#include "../include/tcp_socket.h"
#include "../include/session.h"

int PORT;

int main(int argc, char** argv) {
  int listenfd, connfd;
  struct sockaddr_in server, client;
  socklen_t sin_size = sizeof(struct sockaddr);
  int retval = -1;

  memset(&server, 0, sizeof(server) );

  if (argc != 3) {
    fprintf(stderr, "Please write: ./sensor <id> <password> <port>\n");
    exit(EXIT_FAILURE);
  }

  retval = str_to_port(argv[1], &PORT);
  if (retval == false) {
    fprintf(stderr, "Error: Port number is invalid for user server applications\n");
    exit(EXIT_FAILURE);
  }

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if ( listenfd == -1 ) {
    perror("\nsocket()");
    exit(EXIT_FAILURE);
  }

  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  retval = bind(listenfd, (struct sockaddr*) &server, sizeof(server) );
  if ( retval == -1) {
    perror("\nbind()");
    exit(EXIT_FAILURE);
  }

  if (listen(listenfd, BACKLOG) == -1) {
    perror("\nlisten()");
    exit(EXIT_FAILURE);
  }
  printf("Sensor started!\n");

  // Communicate with client

// Thread communicate with users
// Thread get information from sensors
// Thread control devices
  return 0;
}
