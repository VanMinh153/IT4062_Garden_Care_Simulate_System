#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "session.h"

unsigned numSession = 0;
session_t* shead = NULL;
session_t* stail = NULL;
pthread_mutex_t sessMutex;

/* Add a new node to the end of session_t linked list */
session_t* add_session(session_t* data) {
  session_t* newNode = (session_t*) malloc(sizeof(session_t));
  if (newNode == NULL) {
    fprintf(stderr, "!!Can not malloc!!n");
    return NULL;
  }
  memmove(newNode, data, sizeof(session_t) );
  pthread_mutex_lock(&sessMutex);
  if (stail == NULL) { // add first node
    stail = newNode;
    shead = newNode;
    numSession++;
    pthread_mutex_unlock(&sessMutex);
    return newNode;
  }
  stail->next = newNode;
  newNode->prev = stail;
  stail = newNode;
  numSession++;
  pthread_mutex_unlock(&sessMutex);
  return newNode;
}
/* Delete one node from session_t linked list */
void delete_session(session_t* node) {
  pthread_mutex_lock(&sessMutex);
  if (node->prev == NULL && node->next == NULL) { // delete the only remaining node
    shead = NULL;
    stail = NULL;
    numSession--;
    pthread_mutex_unlock(&sessMutex);
    free(node);
    return;
  } else if (node->prev == NULL) { // delete first node
    shead = node->next;
    shead->prev = NULL;
    numSession--;
    pthread_mutex_unlock(&sessMutex);
    free(node);
    return;
  } else if (node->next == NULL) { // delete final node
    stail = node->prev;
    stail->next = NULL;
    numSession--;
    pthread_mutex_unlock(&sessMutex);
    free(node);
    return;
  } else {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    numSession--;
    pthread_mutex_unlock(&sessMutex);
    free(node);
    return;
  }
}
