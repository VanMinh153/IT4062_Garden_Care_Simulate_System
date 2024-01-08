// UDP Echo Client
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "../Library/utility.h"

#define MSG_SIZE 2048
int SERVER_PORT;

int main(int argc, char **argv) {
  int client_sock;
  char sent[MSG_SIZE], recv[MSG_SIZE];
  int sent_bytes, received_bytes;
  struct sockaddr_in server;
  socklen_t sin_size = sizeof(struct sockaddr);
  int retval;

  if (argc != 3) {
    fprintf(stderr, "Please write: ./client <Server_IP> <Port_Number>\n");
    return -1;
  }
  retval = inet_pton(AF_INET, argv[1], &server.sin_addr);
  if (retval == 0) {
    perror("Warning: Invalid IPv4 address\n");
    return -1;
  } else if (retval == -1){
    perror("Error: inet_pton() run failed");
    return -1;
  }
  if (!str_to_port(argv[2], &SERVER_PORT)) {
    perror("Error: Port number is invalid for user server applications\n");
    return -1;
  }


// Step 1: Construct a UDP socket
  if ( (client_sock = socket(AF_INET, SOCK_DGRAM, 0) ) == -1) {
    perror("\nError: socket() function fail run");
    exit(EXIT_FAILURE);
  }

// Step 2: Define the address of the server
  server.sin_family = AF_INET;
  server.sin_port = htons(SERVER_PORT);

// Step 3: Communicate with clients
  while (1) {
    printf("\nInsert string to send: ");
    memset(sent, 0, MSG_SIZE);
    fgets(sent, MSG_SIZE, stdin);
    if (sent[0] == '\n') {
      printf("Client off\n");
      return 0;
    }

    sent_bytes = sendto(client_sock, sent, strlen(sent), 0, (struct sockaddr*) &server, sin_size);
    if (sent_bytes < 0) {
      perror("\nError: sendto() run fail");
      close(client_sock);
      exit(EXIT_FAILURE);
    }

    received_bytes = recvfrom(client_sock, recv, MSG_SIZE - 1, 0, (struct sockaddr*) &server, &sin_size);
    if (received_bytes < 0) {
      perror("\nError: recv() run fail");
      close(client_sock);
      exit(EXIT_FAILURE);
    }
    recv[received_bytes] = '\0';

    if (recv[0] == '+') {
      printf("Success: ");
      char *pch = strtok(recv+1, " ");
      while (pch != NULL) {
        printf("%s\n", pch);
        pch = strtok(NULL, " ");
      }
    } else if (recv[0] == '-') {
      printf("Fail: %s\n", recv+1);
    } else {
      perror("Error: Unknow message\n");
      close(client_sock);
      return 0;
    }
  }
  close(client_sock);
  return 0;
}






