#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "../include/hw5_utility.h"
#include "../include/tcp_socket.h"

#define MTU 1000000 // ~1MB

int SERVER_PORT;

int main(int argc, char** argv) {
  int client_sock;
  struct sockaddr_in server;
  char send_data[MSG_SIZE];
  char recv_msg[MSG_SIZE], recv_buffer[MSG_SIZE];
  int retval = -1;

  memset(recv_msg, 0, MSG_SIZE);
  memset(recv_buffer, 0, MSG_SIZE);

  if (argc != 3) {
    fprintf(stderr, "Please write: ./client <Server_IP> <Port_Number>\n");
    exit(EXIT_FAILURE);
  }

  retval = inet_pton(AF_INET, argv[1], &server.sin_addr);
  if (retval == 0) {
    fprintf(stderr, "Warning: Invalid IPv4 address\n");
    exit(EXIT_FAILURE);
  } else if (retval == -1) {
    perror("\ninet_pton()");
    exit(EXIT_FAILURE);
  }

  if (!str_to_port(argv[2], &SERVER_PORT)) {
    fprintf(stderr, "Error: Port number is invalid for user server applications\n");
    exit(EXIT_FAILURE);
  }

// step 1: Construct socket
  if ( (client_sock = socket(AF_INET, SOCK_STREAM, 0) ) == -1 ) {
    perror("\nsocket()");
    exit(EXIT_FAILURE);
  }

// step 2: Specify server address
  server.sin_family = AF_INET;
  server.sin_port = htons(SERVER_PORT);

// step 3: Request to connect server
  if (connect(client_sock, (struct sockaddr*) &server, sizeof(struct sockaddr) ) < 0) {
    perror("\nconnect()");
    exit(EXIT_FAILURE);
  }
  get_msg(client_sock, recv_msg, recv_buffer);
  print_msg(recv_msg);
// step 4: Communicate with server
  int userChoice;
  char inpstr[MSG_SIZE];
  const int ARTICLE_MAX = MSG_SIZE - 5;
  char article[ARTICLE_MAX + 1];
  const int NAME_MAX = MSG_SIZE - 5;
  char username[NAME_MAX + 1];

  while (1) {
    userChoice = 0;
    inpstr[0] = '\0';
    printf("\n---------------------------------------------\n");
    printf("\t USER MANAGEMENT PROGRAM\n");
    printf("1. Log in\n");
    printf("2. Post message\n");
    printf("3. Logout\n");
    printf("4. Exit\n");

    while (1) {
      printf("Choose feature [1-4]: ");
      fgets(inpstr, 3, stdin);
      if (inpstr[strlen(inpstr) - 1] != '\n') clear_stdin();
      if (inpstr[1] != '\n') continue;
      int choice = (int) inpstr[0] - (int) '0';
      if (choice < 1 || choice > 4) continue;
      userChoice = choice;
      break;
    }
    switch (userChoice) {
      case 1:
        printf("Typing username: ");
        fgets(username, NAME_MAX + 1, stdin);
        if (username[strlen(username) - 1] != '\n') clear_stdin();
        if (strlen(username) == NAME_MAX) {
          fprintf(stderr, "Over length of message\n");
          break;
        }
        username[strlen(username) - 1] = '\0';

        sprintf(send_data, "USER %s", username);
        send_msg(send_data, client_sock);

        get_msg(client_sock, recv_msg, recv_buffer);
        print_msg(recv_msg);
        break;
      case 2:
        printf("Input content: ");
        fgets(article, ARTICLE_MAX + 1, stdin);
        if (article[strlen(article) - 1] != '\n') clear_stdin();
        if (strlen(article) == ARTICLE_MAX) {
          fprintf(stderr, "Over length of message\n");
          break;
        }
        article[strlen(article) -1] = '\0';

        sprintf(send_data, "POST %s", article);
        send_msg(send_data, client_sock);

        get_msg(client_sock, recv_msg, recv_buffer);
        print_msg(recv_msg);
        break;
      case 3:
        send_msg("BYE", client_sock);

        get_msg(client_sock, recv_msg, recv_buffer);
        print_msg(recv_msg);
        break;
      case 4:
        close(client_sock); return 0;
      default: break;
    }
  }
  return 0;
}
