#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <pthread.h>
#include <errno.h>
#include "../include/utility.h"
#include "../include/tcp_socket.h"
#include "../include/session.h"

#define DATA_ALLOC_SIZE 1000000 // ~1MB

#define ACCOUNT_FILE "account.txt"
#define ACCOUNT_MAX 5000  // Max account number
// NAME_SIZE = 128 have been define in session.h

typedef struct account_t {
  char username[NAME_SIZE];
  bool status;
} account_t;

int PORT;
extern session_t* head;
extern session_t* tail;
account_t* accounts;
unsigned int numAccount = 0;
pthread_mutex_t loginMutex;

void* handle_thread(void* connfd);
int handle_msg(char* message, session_t* session);
int read_account();

int main(int argc, char** argv) {
  int listenfd, connfd;
  struct sockaddr_in server, client;
  char clientIP[INET_ADDRSTRLEN];
  socklen_t sin_size = sizeof(struct sockaddr);
  int retval = -1;
  pthread_t tid;

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

  memset(&server, 0, sizeof(server) );
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
  session_t sessionInfo;

	while (1) {
    if ( (connfd = accept(listenfd, (struct sockaddr*) &client, &sin_size) ) == -1) {
      perror("\naccept()");
      return 0;
    }

    inet_ntop(AF_INET, &client.sin_addr, clientIP, sizeof(clientIP) );
    // Can print client's IP and client's port here
    sessionInfo.hasLogin = false;
    sessionInfo.username[0] = '\0';
    sessionInfo.connfd = connfd;
    memmove(sessionInfo.clientIP, clientIP, INET_ADDRSTRLEN);
    sessionInfo.prev = NULL;
    sessionInfo.next = NULL;
    add_session(&sessionInfo);
    retval = pthread_create(&tid, NULL, &handle_thread, tail);
    if (retval != 0) perror("\npthread_create()");
    printf("$ Number of session: %d. New connection socket id: %d\n", numSession, connfd);
	}

	close(listenfd);
	return 0;
}
/*
@function: Get and handle message from client in a new thread
@parameter: [IN] arg: session's infomation
@return: response message back to the client
*/
void* handle_thread(void* arg) {
  pthread_detach(pthread_self());

  session_t* curSession = (session_t*) arg;
  session_t sessionInfo = *( (session_t*) arg);
  int connfd = sessionInfo.connfd;
  char recv_msg[MSG_SIZE], recv_buffer[MSG_SIZE];
  recv_msg[0] = '\0';
  recv_buffer[0] = '\0';
  int retval = -1;

  send_msg(CONNECT_SUCCESS, connfd);
  while (1) {
    retval = get_msg(connfd, recv_msg, recv_buffer);
    if (retval < 0) {
      if (retval == -3) {
        fprintf(stderr, "Error: messages received exceed the maximum message size\n");
        fprintf(stderr, "Notice: message length is limited to %d characters\n", MSG_SIZE);
        send_msg(MSG_NOT_DETERMINED, connfd);
      } else {
        close(connfd);
        delete_session(curSession);
        pthread_exit(NULL);
      }
    }
    handle_msg(recv_msg, curSession);
  }
}
/*
@function: handle message from client
@parameter: [IN] session: session's infomation
            [IN] message: message from client
@return: response message back to the client
*/
int handle_msg(char* message, session_t* session) {
  session_t* tmp = NULL;
  bool hasLogin = session->hasLogin;
  int connfd = session->connfd;
  char command[5];
  char username[NAME_SIZE+1];
  char type_of_msg[21];
  char overcheck = '\0';
  int retval = -1;
  sscanf(message, "%4s", command);
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

    bool accExist = false;
    for (int i = 0; i < numAccount; i++) {
      if (strcmp(username, accounts[i].username) == 0) {
        accExist = true;
        if (accounts[i].status == false) {
          send_msg(ACCOUNT_LOCKED, connfd);
          return 0;
        }
      }
    }
    if (accExist == false) {
      send_msg(ACCOUNT_NOT_EXIST, connfd);
      return 0;
    }

    tmp = head;
    pthread_mutex_lock(&loginMutex);
    while (tmp->next != NULL) {
      if (strcmp(tmp->username, username) == 0) {
        if (tmp != session) {
          pthread_mutex_unlock(&loginMutex);
          send_msg(ALREADY_SIGN_IN, connfd);
          return 0;
        } else {
          pthread_mutex_unlock(&loginMutex);
          send_msg(LOGGED_IN, connfd);
          return 0;
        }
      }
      tmp = tmp->next;
    }
    session->hasLogin = true;
    memmove(session->username, username, sizeof(username));
    pthread_mutex_unlock(&loginMutex);
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
          Account's infomation have been read successful will be save into accounts array
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
