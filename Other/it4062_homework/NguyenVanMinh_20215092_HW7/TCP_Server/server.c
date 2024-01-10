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

#define DATA_ALLOC_SIZE 1000000 // ~1MB

#define ACCOUNT_FILE "account.txt"
#define ACCOUNT_MAX 5000  // Max account number
#define WAIT_TIME 30000   // 30 000 us, time to wait for handle new connection if server don't have any connection
#define TIME_OUT 3       // 3 ms, timeout when call poll() function
// NAME_SIZE = 128 have been define in session.h

typedef struct account_t {
  char username[NAME_SIZE];
  bool status;
} account_t;
account_t* accounts;
unsigned int numAccount = 0;

extern int qfront;
int PORT;

int handle_msg(char* message, session_t* session);
int read_account();

int main(int argc, char** argv) {
  int listenfd, connfd;
  struct sockaddr_in server, client;
  socklen_t sin_size = sizeof(struct sockaddr);
  int retval = -1;
  char recv_msg[MSG_SIZE];

  recv_msg[0] = '\0';
  memset(&server, 0, sizeof(server) );

  if (argc != 2) {
    fprintf(stderr, "Please write: ./server <Port Number>\n");
    exit(EXIT_FAILURE);
  }

  if (!str_to_port(argv[1], &PORT)) {
    fprintf(stderr, "Error: Port number is invalid for user server applications\n");
    exit(EXIT_FAILURE);
  }

  if ( (listenfd = socket(AF_INET, SOCK_STREAM, 0) ) == -1 ) {
    perror("\nsocket()");
    exit(EXIT_FAILURE);
  }

  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(listenfd, (struct sockaddr*) &server, sizeof(server) ) == -1) {
    perror("\nbind()");
    exit(EXIT_FAILURE);
  }

  if (listen(listenfd, BACKLOG) == -1) {
    perror("\nlisten()");
    exit(EXIT_FAILURE);
  }
  printf("TCP Server started!\n");

  // Communicate with client
  accounts = (account_t*) malloc(ACCOUNT_MAX*sizeof(account_t) );
  numAccount = read_account(accounts);
  printf("Information of %d accounts have been read successful\n", numAccount);

  struct pollfd listenfd_poll;
  session_t* sessfd[CLIENT_MAX];
  struct pollfd fds[CLIENT_MAX];
  int nfds = 0;
  int nready = 0;
  session_t sessInfo;
  int id = 0;

  listenfd_poll.fd = listenfd;
  listenfd_poll.events = POLLIN;

  for (int i = 0; i < CLIENT_MAX; i++) {
    fds[i].fd = -1;
    fds[i].events = POLLIN;
  }

  while (1) {
    while (1) {
      retval = poll(&listenfd_poll, 1, TIME_OUT);
      if (retval == -1) {
        perror("\npoll()1");
        return 1;
      }

      if (listenfd_poll.revents & POLLIN) {
        for (id = 0; id <= CLIENT_MAX; id++)
          if (fds[id].fd == -1) break;

        if (id == CLIENT_MAX) break;
        else {
          connfd = accept(listenfd, (struct sockaddr*) &client, &sin_size);
          if (connfd == -1) {
            perror("\naccept()");
            return 1;
          }
          sessInfo.recv_buffer[0] = '\0';
          sessInfo.hasLogin = false;
          sessInfo.username[0] = '\0';
          sessInfo.connfd = connfd;
          sessInfo.prev = NULL;
          sessInfo.next = NULL;
          sessfd[id] = add_session(&sessInfo);
          fds[id].fd = sessInfo.connfd;
          nfds++;
          send_msg(CONNECT_SUCCESS, fds[id].fd);
        }
      } else break;
    }

    // Get message and response to client
    nready = poll(fds, CLIENT_MAX, TIME_OUT);

    if (nready == 0) continue;
    else if (nready == -1) {
      perror("\npoll()2");
      continue;
    }

    for (id = 0; id < CLIENT_MAX; id++) {
      if (fds[id].revents & POLLIN) {
        connfd = fds[id].fd;
        do {
          retval = get_msg(connfd, recv_msg, sessfd[id]->recv_buffer);
          if (retval < 0) {
            if (retval == -3) {
              fprintf(stderr, "Error: messages received exceed the maximum message size\n");
              fprintf(stderr, "Notice: message length is limited to %d characters\n", MSG_SIZE);
              send_msg(MSG_NOT_DETERMINED, connfd);
            } else if (retval == -2) {
              close(connfd);
              delete_session(sessfd[id]);
              fds[id].fd = -1;
              nfds--;
              break;
            } else {
              fprintf(stderr, "\nget_msg(): Error\n");
              return -1;
            }
          } else handle_msg(recv_msg, sessfd[id]);

          if ( poll(fds + id, 1, 0) > 0) continue;
        } while (sessfd[id]->recv_buffer[0] != '\0');

        nready--;
        if (nready == 0) break;
      }
    }
  }
  fprintf(stderr, "\nServer has error!!");
	close(listenfd);
	return 0;
}
/*
@function: handle message from client
@parameter: [IN] session: session's infomation
            [IN] message: message from client
@return: response message back to the client
*/
int handle_msg(char* message, session_t* session) {
  bool hasLogin = session->hasLogin;
  int connfd = session->connfd;
  char command[5];
  char username[NAME_SIZE+1];
  char type_of_msg[21];
  char overcheck = '\0';
  int retval = -1;
  sscanf(message, "%4s", command); // Get 4 character for the command
  // handle the USER command
  if (strcmp(command, "USER") == 0) {
    snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %%%ds %%c", NAME_SIZE);
    retval = sscanf(message, type_of_msg, username, overcheck);

    if (strlen(username) == NAME_SIZE) {
      send_msg(ACCOUNT_NOT_EXIST, connfd);
      return 0;
    }
    if ( retval < 1 || overcheck != '\0' ) {
      send_msg(ACCOUNT_NOT_EXIST, connfd);
      return 0;
    }
    if (hasLogin == true) {
      send_msg(LOGGED_IN, connfd);
      return 0;
    }

    bool accExitst = false;
    for (int i = 0; i < numAccount; i++) {
      if (strcmp(username, accounts[i].username) == 0) {
        accExitst = true;
        if (accounts[i].status == false) {
          send_msg(ACCOUNT_LOCKED, connfd);
          return 0;
        }
      }
    }
    if (accExitst == false) {
      send_msg(ACCOUNT_NOT_EXIST, connfd);
      return 0;
    }

    session->hasLogin = true;
    memmove(session->username, username, sizeof(username));
    // Can print user logged in successful here
    send_msg(LOGIN_SUCCESS, connfd);
    return 0;
  // handle the POST command
  } else if (strcmp(command, "POST") == 0) {
    if (hasLogin == false) {
      send_msg(NOT_LOGGED_IN, connfd);
      return 0;
    }
    // Can print post content here
    send_msg(POST_SUCCESS, connfd);
    return 0;
  // handle the BYE command
  } else if (strcmp(command, "BYE") == 0) {
    if (hasLogin == false) {
      send_msg(NOT_LOGGED_IN, connfd);
      return 0;
    }
    session->hasLogin = false;
    *session->username = '\0';
    send_msg(LOGOUT_SUCCESS, connfd);
    return 0;
  } else {
    send_msg(MSG_NOT_DETERMINED, connfd);
    return 0;
  }
}
/*
@function: Read ACCOUNT_FILE and save account infomation to account_t accounts[]
          Account's infomation have been read successful will be save into acounts array
@return: number account have been read successful
*/
int read_account() {
  int fd = open(ACCOUNT_FILE, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "Cannot open file %s: %s", ACCOUNT_FILE, strerror(errno) );
    exit(1);
  }

  char* buffer = (char*) malloc(DATA_ALLOC_SIZE);
  int read_bytes;
  int retval = -1;

  int numAccount = 0;
  int line = 0;
  char* token = NULL;
  char username[NAME_SIZE+1];
  char status = '\0';
  char overcheck = '\0';
  char type_of_line[17];
  retval = snprintf(type_of_line, sizeof(type_of_line), "%%%ds %%c %%c", NAME_SIZE);
  if (retval < 0) perror("\nsnprintf()");
  else if ( (unsigned int) retval > sizeof(type_of_line) - 1)
    fprintf(stderr, "Warning: type_of_line string length exceeded\n");


  do {
    read_bytes = read(fd, buffer, DATA_ALLOC_SIZE - 1);
    if (read_bytes < 0) {
      perror("\nread()");
      free(buffer);
      exit(1);
    }
    if (read_bytes == 0) break;
    buffer[read_bytes] = '\0';

    token = strtok(buffer, "\n");
    status = '\0';
    username[0] = '\0';
    while (token != NULL) {
      line ++;
      overcheck = '\0';
      retval = sscanf(token, type_of_line, username, &status, &overcheck);
      if (strlen(username) == NAME_SIZE) {
        fprintf(stderr, "Note: the account name cannot be more than %d characters\n", NAME_SIZE);
        fprintf(stderr, "Hint: change NAME_SIZE in the server.c to be able to have more characters\n");
        fprintf(stderr, "Warning: this line will be ignore\n");
        fprintf(stderr, "%7d | %s\n", line, token);
        token = strtok(NULL, "\n");
        continue;
      }
      if ( retval < 2 || (overcheck != '\0') || (status != '0' && status != '1') ) {
        fprintf(stderr, "Warning: cannot parsing, this line will be ignore\n");
        fprintf(stderr, "%7d | %s\n", line, token);
        token = strtok(NULL, "\n");
        continue;
      }
      memmove(accounts[numAccount].username, username, strlen(username) + 1);
      accounts[numAccount].status = (status == '1');
      numAccount++;
      if (numAccount == ACCOUNT_MAX) {
        fprintf(stderr, "Note: cannot save more than %d accounts\n", ACCOUNT_MAX);
        fprintf(stderr, "Hint: change ACCOUNT_MAX in the server.c to be able to save more accounts\n");
        fprintf(stderr, "Warning: stopped reading the file %s from line %d\n", ACCOUNT_FILE, line);
        free(buffer);
        close(fd);
        return numAccount;
      }
      token = strtok(NULL, "\n");
    }
  } while (read_bytes > 0);
  // Can print account infomation here
  free(buffer);
  close(fd);
  return numAccount;
}

