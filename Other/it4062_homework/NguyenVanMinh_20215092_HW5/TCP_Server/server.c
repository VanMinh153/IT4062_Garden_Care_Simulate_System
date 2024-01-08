#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <errno.h>
#include "../include/hw5_utility.h"
#include "../include/tcp_socket.h"

#define BACKLOG 20
#define DATA_ALLOC_SIZE 1000000 // ~1MB

#define ACCOUNT_FILE "account.txt"
#define NAME_SIZE 128     // Max length of username is 127 character
#define ACCOUNT_MAX 5000  // Max account number

typedef struct account_t {
  char username[NAME_SIZE];
  bool status;
} account_t;
typedef struct session_t {
  bool hasLogin;
  char username[NAME_SIZE];
  int conn_sock;
} session_t;
int PORT;

void sig_chld(int signo);
char* handle_msg(account_t* accounts, int numAccounts, session_t* session, char* message);
int read_account(account_t* accounts);

int main(int argc, char** argv) {
  int listen_sock, conn_sock;
  struct sockaddr_in server, client;
  char clientIP[INET_ADDRSTRLEN];
  socklen_t sin_size = sizeof(struct sockaddr);
  pid_t pid;
  char recv_msg[MSG_SIZE], recv_buffer[MSG_SIZE];

  memset(recv_msg, 0, MSG_SIZE);
  memset(recv_buffer, 0, MSG_SIZE);

  if (argc != 2) {
    fprintf(stderr, "Please write: ./server <Port Number>\n");
    exit(EXIT_FAILURE);
  }

  if (!str_to_port(argv[1], &PORT)) {
    fprintf(stderr, "Error: Port number is invalid for user server applications\n");
    exit(EXIT_FAILURE);
  }

// step 1: Construct a TCP socket to listen connection request
  if ( (listen_sock = socket(AF_INET, SOCK_STREAM, 0) ) == -1 ) {
    perror("\nsocket()");
    exit(EXIT_FAILURE);
  }

// step 2: bind address to socket
  memset(&server, 0, sizeof(server) );
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(listen_sock, (struct sockaddr*) &server, sizeof(server) ) == -1) {
    perror("\nbind()");
    exit(EXIT_FAILURE);
  }

// step 3: listen request from client
  if (listen(listen_sock, BACKLOG) == -1) {
    perror("\nlisten()");
    exit(EXIT_FAILURE);
  }
  printf("TCP Server started!\n");
// Establish a signal handler to catch SIGCHLD
	signal(SIGCHLD, sig_chld);
// step 4: Communicate with client
  account_t* accounts = (account_t*) malloc(ACCOUNT_MAX*sizeof(account_t) );
  int numAccounts = read_account(accounts);
  printf("Information of %d accounts have been read successful\n", numAccounts);

	while(1){
    if ( (conn_sock = accept(listen_sock, (struct sockaddr*) &client, &sin_size) ) == -1) {
			if (errno == EINTR) continue;
			else {
				perror("\naccept()");
				return 0;
			}
    }
    inet_ntop(AF_INET, &client.sin_addr, clientIP, sizeof(clientIP) );
    // Can print client's IP and client's port here
		pid = fork();
		if (pid == 0) {
      // child process
      close(listen_sock);

			session_t session;
			session.hasLogin = false;
			session.username[0] = '\0';
			session.conn_sock = conn_sock;
			int retval = -1;

			send_msg(CONNECT_SUCCESS, conn_sock);
			while (1) {
        retval = get_msg(conn_sock, recv_msg, recv_buffer);
        if (retval == -2) {
          close(conn_sock);
          exit(0);
        }
        handle_msg(accounts, numAccounts, &session, recv_msg);
			}
		}
    // end of child process
		close(conn_sock);
	}
	close(listen_sock);
	return 0;
}
/*
@function: Handler process signal
@parameter [IN] signo
*/
void sig_chld(int signo){
	pid_t pid;
	int stat;

	while ( (pid = waitpid(-1, &stat, WNOHANG) ) > 0);
    // Can print child process pid here
}
/*
@function: handle message from client
@parameter [IN] accounts: array containing account information
           [IN] numAccounts: number of accounts
           [IN] session: session's infomation
           [IN] message: message from client
@return: response message back to the client
*/
char* handle_msg(account_t* accounts, int numAccounts, session_t* session, char* message) {
  bool hasLogin = (*session).hasLogin;
  int conn_sock = (*session).conn_sock;
  char command[5];
  char username[NAME_SIZE+1];
  char type_of_msg[21];
  char overcheck = '\0';
  int retval = -1;
  sscanf(message, "%4s", command);
// handle the USER command
  if (strcmp(command, "USER") == 0) {
    // check username
    snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %%%ds %%c", NAME_SIZE);
    retval = sscanf(message, type_of_msg, username);
    if (strlen(username) == NAME_SIZE) {
      send_msg(ACCOUNT_NOT_EXIST, conn_sock);
      return ACCOUNT_NOT_EXIST;
    }
    if ( retval < 1 || (overcheck != '\0') ) {
      send_msg(MSG_NOT_DETERMINED, conn_sock);
      return MSG_NOT_DETERMINED;
    }

    bool accExitst = false;
    if (hasLogin == true) {
      send_msg(LOGGED_IN, conn_sock);
      return LOGGED_IN;
    }

    for (int i = 0; i < numAccounts; i++) {
      if (strcmp(username, accounts[i].username) == 0) {
        accExitst = true;
        if (accounts[i].status == true) {
          (*session).hasLogin = true;
          memmove((*session).username, username, sizeof(username));
          // Can print user logged in successful here
          send_msg(LOGIN_SUCCESS, conn_sock);
          return LOGIN_SUCCESS;
        } else {
          send_msg(ACCOUNT_LOCKED, conn_sock);
          return ACCOUNT_LOCKED;
        }
      }
    }
    if (accExitst == false) {
      send_msg(ACCOUNT_NOT_EXIST, conn_sock);
      return ACCOUNT_NOT_EXIST;
    }
// handle the POST command
  } else if (strcmp(command, "POST") == 0) {
    if (hasLogin == false) {
      send_msg(NOT_LOGGED_IN, conn_sock);
      return NOT_LOGGED_IN;
    }
    // Can print post content here
    send_msg(POST_SUCCESS, conn_sock);
    return POST_SUCCESS;
// handle the BYE command
  } else if (strcmp(command, "BYE") == 0) {
    if (hasLogin == false) {
      send_msg(NOT_LOGGED_IN, conn_sock);
      return NOT_LOGGED_IN;
    }
    (*session).hasLogin = false;
    send_msg(LOGOUT_SUCCESS, conn_sock);
    return LOGOUT_SUCCESS;
  } else {
    send_msg(MSG_NOT_DETERMINED, conn_sock);
    return MSG_NOT_DETERMINED;
  }
  return NULL;
}
/*
@function: read ACCOUNT_FILE and save account infomation to account_t accounts[]
@parameter [OUT] accounts: save account infomation have been read successful
@return: number account have been read successful
*/
int read_account(account_t* accounts) {
  int fd = open(ACCOUNT_FILE, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "Cannot open file %s: %s", ACCOUNT_FILE, strerror(errno) );
    exit(1);
  }

  char* buffer = (char*) malloc(DATA_ALLOC_SIZE);
  int read_bytes;
  int retval = -1;

  int numAccounts = 0;
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
      memmove(accounts[numAccounts].username, username, strlen(username) + 1);
      accounts[numAccounts].status = (status == '1');
      numAccounts++;
      if (numAccounts == ACCOUNT_MAX) {
        fprintf(stderr, "Note: cannot save more than %d accounts\n", ACCOUNT_MAX);
        fprintf(stderr, "Hint: change ACCOUNT_MAX in the server.c to be able to save more accounts\n");
        fprintf(stderr, "Warning: stopped reading the file %s from line %d\n", ACCOUNT_FILE, line);
        free(buffer);
        close(fd);
        return numAccounts;
      }
      token = strtok(NULL, "\n");
    }
  } while (read_bytes > 0);
  // Can print account infomation here
  free(buffer);
  close(fd);
  return numAccounts;
}
