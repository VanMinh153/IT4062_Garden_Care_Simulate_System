#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "utility.h"

/*
@function: Convert a string to port number available for user server applications.
          Available port number for user server applications is [1024, 49151]
@parameter [IN] str: a string containing port number
           [OUT] port: port number retrieve from str
@return: true if success
         false if fail
*/
bool str_to_port(char* str, int* port) {
  int n = 0;
  char overcheck = '\0';
  sscanf(str, "%5d %c", &n, &overcheck);
  if (overcheck != '\0') return false;
  if (n >= 1024 && n <= 49151) {
    *port = n;
    return true;
  }
  return false;
}
/* Clear stdin buffer */
void clear_stdin() {
  char chr;
  while ( (chr = getchar() ) != EOF && chr != '\n');
}
/*
@function: Removes whitespace at the beginning and end of a string
@parameter [IN]: a string will to handle
@return: pointer to string after handle
         NULL if string only has space or tab character
*/
char* clearstr(char* str) {
  char* p1 = str;
  char* p2 = str + strlen(str);

  while (*p1 == ' ' || *p1 == '\t') p1++;
  if (p1 == p2) return NULL;
  p2--;
  while (*p2 == ' ' || *p2 == '\t') {
    if (p2 == p1) return NULL;
    p2--;
  }
  p2++;
  *p2 = '\0';
  return p1;
}
