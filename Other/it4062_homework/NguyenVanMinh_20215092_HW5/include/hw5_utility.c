#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "hw5_utility.h"

char strTime[22];

/*
@function: get current time with form "[dd/mm/yyyy hh:mm:ss]"
@return strTime: pointer to string "[dd/mm/yyyy hh:mm:ss]"
*/
char* get_time() {
  time_t currentTime;
  struct tm *tmTime;
  time(&currentTime);
  tmTime = localtime(&currentTime);
  strftime(strTime, sizeof(strTime), "[%d/%m/%Y %H:%M:%S]", tmTime);
  return strTime;
}
/*
@function: Convert a string to port number available for user server applications.
          Available port number for user server applications is [1024, 49151]
@parameter [IN] str: A string containing port number
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
