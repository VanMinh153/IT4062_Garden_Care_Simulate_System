#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../Library/utility.h"

#define BACKLOG 2 /* Number of allowed connections */
#define MSG_SIZE 50000
#define PATH_SIZE 4096
#define FILENAME_SIZE 256
#define DATA_ALLOC_SIZE 100000000 // 1e8
#define DELIMITER "\\r\\n"
int PORT;

int main(int argc, char** argv) {
  FILE* log = fopen("./TCP_Server/log_20215092.txt", "w");
  int listen_sock, conn_sock;
  char sent_data[MSG_SIZE], recv_data[MSG_SIZE], recv_msg[MSG_SIZE];
  unsigned int sent_bytes, received_bytes;
  struct sockaddr_in server, client;
  char clientIP[INET_ADDRSTRLEN];
  int clientPort;
  socklen_t sin_size = sizeof(struct sockaddr);

  if (argc != 3) {
    fprintf(stderr, "Please write: ./server <Port Number> <Directory Name>\n");
    return -1;
  }

  if (!str_to_port(argv[1], &PORT)) {
    fprintf(stderr, "Error: Port number is invalid for user server applications\n");
    return -1;
  }

  const char* dirPath = argv[2];
// Create storage folder if it not exist
    if (access(dirPath, F_OK) != 0)
      if (mkdir(dirPath, 0755) != 0) {
        fprintf(stderr, "Failed to create the directory %s", dirPath);
        perror("\nError");
        exit(1);
      }

// step 1: Construct a TCP socket to listen connection request
  if ( (listen_sock = socket(AF_INET, SOCK_STREAM, 0) ) == -1 ) {
    perror("\nError");
    exit(EXIT_FAILURE);
  }

// step 2: bind address to socket
  memset(&server, 0, sizeof(server) );
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(listen_sock, (struct sockaddr*) &server, sizeof(server) ) == -1) {
    perror("\nError");
    exit(EXIT_FAILURE);
  }

// step 3: listen request from client
  if (listen(listen_sock, BACKLOG) == -1) {
    perror("\nError");
    exit(EXIT_FAILURE);
  }
  printf("TCP Server started!\n");

// step 4: Communicate with client
////
////
begin_state:
  while (1) {
    fclose(log);
    log = fopen("./TCP_Server/log_20215092.txt", "a");
    memset(sent_data, 0, MSG_SIZE);
    char welcomeMsg[] = "+OK Welcome to file server";
    char readyMsg[] = "+OK Please send file";
    char successMsg[] = "+OK Successful upload";
    char closeMsg[] = "-Error Connection closed while uploading file";
    if ( (conn_sock = accept(listen_sock, (struct sockaddr*) &client, &sin_size) ) == -1) {
      perror("\nError");
      exit(EXIT_FAILURE);
    }
    inet_ntop(AF_INET, &client.sin_addr, clientIP, sizeof(clientIP) );
    clientPort = ntohs(client.sin_port);

    fprintf(log, "%s $ %s:%d $ %s\n", getTime(), clientIP, clientPort, welcomeMsg);
//

    do {
      received_bytes = recv(conn_sock, recv_data, MSG_SIZE - 1, 0);
      if (received_bytes < 0) perror("\nError");
      else if (received_bytes == 0) {
        printf("[%s:%d] > Connection closed.\n", clientIP, clientPort);
        goto begin_state;
      }
      else {
        recv_data[received_bytes] = '\0';
        memset(recv_msg, 0, MSG_SIZE);
        char* optr = recv_data;
        char* ptr = strstr(recv_data, DELIMITER);
        while (1) {
          if (ptr == NULL) {
            memmove(recv_msg+strlen(recv_msg), optr, strlen(optr) );
            break;
          } else {
            memmove(recv_msg+strlen(recv_msg), optr, ptr - optr);
            if (strstr(recv_msg, "UPLD ") == recv_msg) goto ready_state; // -> request processing
recv_state: // receive other request
            memset(recv_msg, 0, MSG_SIZE);
            optr = ptr + strlen(DELIMITER);
            ptr = strstr(ptr + 1, DELIMITER);
          }
        }
      }
    } while (received_bytes != 0);

ready_state:
    sent_bytes = send(conn_sock, readyMsg, sizeof(readyMsg), 0);
    if (sent_bytes < 0) perror("\nError");
    fprintf(log, "%s $ %s:%d $ %s $ %s\n", getTime(), clientIP, clientPort, recv_data, readyMsg);

//
    char command[21];
    char opath[PATH_SIZE];
    memset(command, 0, 21);
    memset(opath, 0, PATH_SIZE);
    unsigned long filelen = 0;
    sscanf(recv_msg,"%s %s %ld", command, opath, &filelen);

    char* optr = opath;
    char* ptr = strtok(opath, "/");
    while (ptr != NULL) {
      optr = ptr;
      ptr = strtok(NULL, "/");
    }

    char* filename = optr;
    FILE *fileptr = NULL;
    char path[PATH_SIZE];

    snprintf(path, PATH_SIZE,"./%s/%s", dirPath, filename);
    fileptr = fopen(path, "wb");
    if (fileptr == NULL) {
      perror("\nError");
      exit(1);
    }
    unsigned long idx = 0;
    unsigned long overlen = 0;
    char* filedata = (char*) malloc(DATA_ALLOC_SIZE);
    while (1) {
      received_bytes = recv(conn_sock, filedata, DATA_ALLOC_SIZE, 0);
      if (received_bytes < 0) perror("\nError");
      else if (received_bytes == 0) {
        printf("[%s:%d] > Connection closed.\n", clientIP, clientPort);
        fprintf(log, "%s $ %s:%d $ %s $ %s\n", getTime(), clientIP, clientPort, recv_data, closeMsg);
        goto begin_state;
      }

      idx += received_bytes;
      if (idx > filelen) {
        fwrite(filedata, 1, filelen + received_bytes - idx , fileptr);
        overlen = idx - filelen;
        fprintf(stderr, "Received over %ld bytes from Client\n", overlen);
        break;
      }
      fwrite(filedata, 1, received_bytes, fileptr);
      if (idx == filelen) break;
    }
    fclose(fileptr);
    free(filedata);
// File upload successful
    sent_bytes = send(conn_sock, successMsg, sizeof(successMsg), 0);
    if (sent_bytes < 0) perror("\nError");
    fprintf(log, "%s $ %s:%d $ %s $ %s\n", getTime(), clientIP, clientPort, recv_data, successMsg);
  goto recv_state;
  close(conn_sock);
}
  close(listen_sock);
  return 0;
}

