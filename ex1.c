#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
// #include "../include/utility.h"
// #include "../include/tcp_socket.h"
// #include "../include/session.h"
// #include "../include/simulator.h"

int main() {
  char str[] = "Hello_World!";
  char str2[5] = "IT";
  char c = '\0';
  int retval = -1;
  retval = sscanf(str, "%*s%c", 3, str2, &c);
  printf("%s\n", str2);
  printf("%c\n", c);
  printf("%d\n", retval);
  // printf("size = %d\n", sizeof(struct in_addr) );
  return 0;
}