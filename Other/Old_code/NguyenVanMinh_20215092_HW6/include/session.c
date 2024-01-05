#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "session.h"

unsigned numSession = 0;
session_t* head = NULL;
session_t* tail = NULL;
pthread_mutex_t sessionsMutex;

/* Add a new node to the end of session_t linked list */
void add_session(session_t* data) {
  session_t* newNode = (session_t*) malloc(sizeof(session_t));
  memmove(newNode, data, sizeof(session_t) );
  pthread_mutex_lock(&sessionsMutex);
  if (tail == NULL) { // add first node
    tail = newNode;
    head = newNode;
    numSession++;
    pthread_mutex_unlock(&sessionsMutex);
    return;
  }
  tail->next = newNode;
  newNode->prev = tail;
  tail = newNode;
  numSession++;
  pthread_mutex_unlock(&sessionsMutex);
}
/* Delete one node from session_t linked list */
void delete_session(session_t* node) {
  pthread_mutex_lock(&sessionsMutex);
  if (node->prev == NULL && node->next == NULL) { // delete the only remaining node
    head = NULL;
    tail = NULL;
    free(node);
    numSession--;
    pthread_mutex_unlock(&sessionsMutex);
    return;
  }
  if (node->prev == NULL) { // delete first node
    node->next->prev = NULL;
    head = node->next;
    free(node);
    numSession--;
    pthread_mutex_unlock(&sessionsMutex);
    return;
  } else if (node->next == NULL) { // delete final node
    node->prev->next = NULL;
    tail = node->prev;
    free(node);
    numSession--;
    pthread_mutex_unlock(&sessionsMutex);
    return;
  } else {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    free(node);
    numSession--;
    pthread_mutex_unlock(&sessionsMutex);
    return;
  }
}
