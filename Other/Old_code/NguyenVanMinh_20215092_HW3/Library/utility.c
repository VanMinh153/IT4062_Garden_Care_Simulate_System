#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

char strTime[30];

// convert a string to port number available for user server applications
bool str_to_port(const char* str, int* port) {
  int n;
  if (strlen(str) >= 5) return false;
  sscanf(str, "%d", &n);
  if (n >= 1024 && n <= 49151) {
    *port = n;
    return true;
  }
  return false;
}
// get current time. Support for make log_file
char* getTime() {
    time_t currentTime;
    struct tm *tmTime;
    time(&currentTime);
    tmTime = localtime(&currentTime);
    strftime(strTime, sizeof(strTime), "[%d/%m/%Y %H:%M:%S]", tmTime);
    return strTime;
}
