#include <stdbool.h>
#include <arpa/inet.h>
#ifndef SESSION_H
#define SESSION_H

#define NAME_SIZE 128     // Max length of username is 127 character

extern unsigned int numSession;
struct session_t {
  bool hasLogin;
  char username[NAME_SIZE];
  int connfd;
  char clientIP[INET_ADDRSTRLEN];
  struct session_t* prev;
  struct session_t* next;
};
typedef struct session_t session_t;
extern session_t* head;
extern session_t* tail;

void add_session(session_t* data);

void delete_session(session_t* node);

#endif // SESSION_H
