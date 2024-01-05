#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include "../Library/utility.h"

#define MSG_SIZE 2048
#define PATH_SIZE 4096
#define MTU 10000000 // 1e7
#define DELIMITER "\\r\\n"

int SERVER_PORT;

int main(int argc, char** argv) {
  int client_sock;
  char sent_data[MSG_SIZE], recv_data[MSG_SIZE];
  unsigned int sent_bytes, received_bytes;
  struct sockaddr_in server;
  int retval = -1;

  if (argc != 3) {
    fprintf(stderr, "Please write: ./client <Server_IP> <Port_Number>\n");
    return -1;
  }

  retval = inet_pton(AF_INET, argv[1], &server.sin_addr);
  if (retval == 0) {
    fprintf(stderr, "Warning: Invalid IPv4 address\n");
    return -1;
  } else if (retval == -1){
    perror("\nError");
    return -1;
  }

  if (!str_to_port(argv[2], &SERVER_PORT)) {
    fprintf(stderr, "Error: Port number is invalid for user server applications\n");
    return -1;
  }

// step 1: Construct socket
  if ( (client_sock = socket(AF_INET, SOCK_STREAM, 0) ) == -1 ) {
    perror("\nError");
    exit(EXIT_FAILURE);
  }

// step 2: Specify server address
  server.sin_family = AF_INET;
  server.sin_port = htons(SERVER_PORT);

// step 3: Request to connect server
  if (connect(client_sock, (struct sockaddr*) &server, sizeof(struct sockaddr) ) < 0) {
    perror("\nError");
    exit(EXIT_FAILURE);
  }

// step 4: Communicate with server
  printf(">Welcome to file transfer server<\n");
  while (1) {
    printf("Enter command: ");
    memset(sent_data, 0, MSG_SIZE);
    fgets(sent_data, MSG_SIZE-sizeof(DELIMITER), stdin);
    if (sent_data[0] == '\n') {
      close(client_sock);
      exit(0);
    }
    retval = checkCommand(sent_data);
    if (retval == -1) {
      fprintf(stderr, "Command not found\n");
      continue;
    }
    if (retval < 0) continue;

    char command[21];
    char path[PATH_SIZE];
    memset(command, 0, 21);
    memset(path, 0, PATH_SIZE);
    unsigned long filelenToSend = 0;
    sscanf(sent_data,"%s %s %ld", command, path, &filelenToSend);

    memmove(sent_data+strlen(sent_data)-1, DELIMITER, sizeof(DELIMITER));
    sent_bytes = send(client_sock, sent_data, strlen(sent_data), 0);
    if(sent_bytes < 0) {
      perror("\nError");
      close(client_sock);
      exit(1);
    }
// receive ready stage from server
    received_bytes = recv(client_sock, recv_data, MSG_SIZE - 1, 0);
    if (received_bytes < 0) {
      perror("\nError");
      close(client_sock);
      exit(1);
    }
    recv_data[received_bytes] = '\0';

    if (recv_data[0] == '+') {
      printf(">Success: %s<\n", recv_data+3);
    } else if (recv_data[0] == '-') {
      printf("Fail: %s\n", recv_data+6);
    } else {
      fprintf(stderr, "Error: Unknown message\n");
      close(client_sock);
      return 0;
    }

// upload file
    FILE* fileptr = fopen(path, "rb");
    if (fileptr == NULL) perror("\nError");

    printf("~~~File uploading~~~\n");
    unsigned long idx = 0;
    char* filedata = (char*) malloc(MTU);
    while (1) {
      unsigned int send_size = MTU;
      if (idx + MTU > filelenToSend) send_size = filelenToSend - idx;
      fread(filedata, send_size, 1, fileptr);
      sent_bytes = send(client_sock, filedata, send_size, 0);
      if(sent_bytes < 0) {
        perror("\nError");
        close(client_sock);
        exit(1);
      }
      idx += sent_bytes;
      if (idx == filelenToSend) break;
    }

// receive upload successful stage from server
    received_bytes = recv(client_sock, recv_data, MSG_SIZE - 1, 0);
    if (received_bytes < 0) {
      perror("\nError");
      close(client_sock);
      exit(1);
    }
    recv_data[received_bytes] = '\0';

    if (recv_data[0] == '+') {
      printf(">Success: %s<\n", recv_data+3);
    } else if (recv_data[0] == '-') {
      printf("Fail: %s\n", recv_data+6);
    } else {
      fprintf(stderr, "Error: Unknow message\n");
      close(client_sock);
      return 0;
    }

  }

  close (client_sock);
  return 0;
}
