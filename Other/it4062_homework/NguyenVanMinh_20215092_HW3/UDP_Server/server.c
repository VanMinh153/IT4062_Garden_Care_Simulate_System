// UDP Echo Server
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../Library/utility.h"
#include "../Library/resolver.h"

#define MSG_SIZE 2048
int PORT;

int main(int argc, char **argv) {
  FILE* log = fopen("./UDP_SERVER/log_20215092.txt", "w");
  int server_sock;
  char sent[MSG_SIZE], recv[MSG_SIZE];
  int sent_bytes, received_bytes;
  struct sockaddr_in server, client;
  socklen_t sin_size;

  if (argc != 2) {
    fprintf(stderr, "Please write: ./server <Port_Number>\n");
    return -1;
  }
  if (!str_to_port(argv[1], &PORT)) {
    perror("Error: Port number is invalid for user server applications\n");
    return -1;
  }

// Step 1: Construct a UDP socket
  if ( (server_sock=socket(AF_INET, SOCK_DGRAM, 0) ) == -1) {
    perror("\nError: socket() has failed");
    exit(EXIT_FAILURE);
  }

// Step 2: Bind address to socket
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  bzero(&(server.sin_zero), 8); // zero the rest of the structure

  if (bind(server_sock, (struct sockaddr*) &server, sizeof(struct sockaddr) ) == -1) {
    perror("\nError: bind() has failed");
    exit(EXIT_FAILURE);
  }
  printf("UDP Server started!\n");

// Step 3: communicate with clients
  while (1) {
    log = fopen("./UDP_SERVER/log_20215092.txt", "a");
    void *info = NULL;
    int retval = -1;
    memset(sent, 0, MSG_SIZE);
    char notFoundMsg[] = "-Not found infomation";
    char hasErrorMsg[] = "-Server can not get any infomation";
    char invalidIPMsg[] = "-Invalid IPv4 address";

    sin_size = sizeof(struct sockaddr);
    received_bytes = recvfrom(server_sock, recv, MSG_SIZE - 1, 0, (struct sockaddr*) &client, &sin_size);
    if (received_bytes < 0)
      perror("\nError: recvfrom() has failed");
    else{
      if (recv[received_bytes-1] == '\n') recv[received_bytes-1] = '\0';
      else recv[received_bytes] = '\0';
      retval = resolver(recv, &info);
    }

    if (retval == 0) { // get IPv4 address
      struct addrinfo *rs = NULL;
      struct sockaddr_in *ptrAddr = NULL;

      sent[0] = '+';
      for (rs = (struct addrinfo*) info; rs != NULL; rs = rs->ai_next) {
        char strIP[INET_ADDRSTRLEN];
        ptrAddr = (struct sockaddr_in*) rs->ai_addr;
        inet_ntop(AF_INET, &ptrAddr->sin_addr, strIP, sizeof(strIP) );
        strncat(sent, strIP, MSG_SIZE-1 - strlen(sent) );
        sent[strlen(sent)] = ' ';
      }
      sent_bytes = sendto(server_sock, sent, sizeof(sent), 0, (struct sockaddr*) &client, sin_size);
      if (sent_bytes < 0) perror("\nError: sendto() has failed");
      fprintf(log, "%s $ %s $ %s\n", getTime(), recv, sent);
      freeaddrinfo(info);

    } else if (retval == 1) { // get Domain name
      sent[0] = '+';
      strncat(sent, info, MSG_SIZE - 2);
      sent_bytes = sendto(server_sock, sent, sizeof(sent), 0, (struct sockaddr*) &client, sin_size);
      if (sent_bytes < 0) perror("\nError: sendto() has failed");
      free(info);
      fprintf(log, "%s $ %s $ %s\n", getTime(), recv, sent);

    } else if (retval == -1) { // Get error
      sent_bytes = sendto(server_sock, hasErrorMsg, sizeof(hasErrorMsg), 0, (struct sockaddr*) &client, sin_size);
      if (sent_bytes < 0) perror("\nError: sendto() has failed");
      fprintf(log, "%s $ %s $ -resolver() can not get any infomation\n", getTime(), recv);

    } else if (retval == -2) { // Not found infomation
      sent_bytes = sendto(server_sock, notFoundMsg, sizeof(notFoundMsg), 0, (struct sockaddr*) &client, sin_size);
      if (sent_bytes < 0) perror("\nError: sendto() has failed");
      fprintf(log, "%s $ %s $ -Not found infomation\n", getTime(), recv);

    } else if (retval == -3) { // Invalid IPv4 address
      sent_bytes = sendto(server_sock, invalidIPMsg, sizeof(invalidIPMsg), 0, (struct sockaddr*) &client, sin_size);
      if (sent_bytes < 0) perror("\nError: sendto() has failed");
      fprintf(log, "%s $ %s $ -Not found infomation\n", getTime(), recv);
    }
    fclose(log);
  }

  close(server_sock);
  return 0;
}





