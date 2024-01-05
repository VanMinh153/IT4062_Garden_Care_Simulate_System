#include <stdbool.h>
#include <arpa/inet.h>
#include "tcp_socket.h"
#ifndef SESSION_H
#define SESSION_H

#define NAME_SIZE 128     // Max length of username is 127 character
#define CLIENT_MAX 1024    // Client max in one thread

extern unsigned int numSession;
struct session_t {
  char recv_buffer[MSG_SIZE];
  bool hasLogin;
  char username[NAME_SIZE];
  int connfd;
  struct session_t* prev;
  struct session_t* next;
};
typedef struct session_t session_t;
extern session_t* shead;
extern session_t* stail;

session_t* add_session(session_t* data);

void delete_session(session_t* node);

#endif // SESSION_H
