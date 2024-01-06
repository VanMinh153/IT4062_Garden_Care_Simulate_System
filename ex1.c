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
  char str[] = "Hello_World! _ 22 33 ";
  char str2[25] = "IT";
  char c = '\0';
  int retval = -1;
  int i1 = 0, i2 = 0, i3 = 0;
  // retval = sscanf(str, "%s %d %d %d%c", str2, &i1, &i2, &i3, &c);
  // printf("%d\n", retval);
  // printf("%s %d %d %d %c\n", str2, i1, i2, i3, c);

  char str3[] = "12i2";
  retval = atoi(str3);
  printf("retval = %d\n", retval);

  retval = sscanf(str3, "%d", &i1);
  printf("retval = %d\n", retval);
  printf("%d\n", i1);
  return 0;
}