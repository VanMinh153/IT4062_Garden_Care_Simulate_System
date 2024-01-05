#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#define PATH_SIZE 4096
char strTime[30];

// convert a string to port number available for user server applications
bool str_to_port(const char* str, int* port) {
  int n = 0;
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

/*
@function checkCommand: check command UPLD <Filename> <File Length>
@parameter input: command
@return: 0 if Success
        -1 if Command not found
        -2 if Missing filename operand
        -3 if Missing file length operand or file length is 0
*/
int checkCommand(const char* input) {
  char* str = strdup(input);
  char command[21];
  char path[PATH_SIZE];
  char checkOver[4];
  long long filelenToSend = -1;
  command[0] = '\0';
  path[0] = '\0';
  checkOver[0] = '\0';
  sscanf(str,"%20s %4095s %lld %s", command, path, &filelenToSend, checkOver);


  if (strcmp(command, "UPLD") != 0) return -1; // command not found
  if (strlen(path) == 0) {
      fprintf(stderr, "ULDP: Missing filename operand\n");
      return -2;
  }
  if (filelenToSend < 0) {
      fprintf(stderr, "ULDP: Missing file length operand\n");
      return -3;
  }
  if (filelenToSend == 0) {
      fprintf(stderr, "UPLD: Nothing to upload\n");
      return -3;
  }

  FILE* fileptr = fopen(path, "rb");
  if (fileptr == NULL) {
    perror("\nError");
    return -2;
  }
  unsigned long filelen = 0;
  fseek(fileptr, 0, SEEK_END);
  filelen = ftell(fileptr);
  fclose(fileptr);
  if (filelen < 0) {
    perror("\nError");
    return -2;
  }
  if (filelenToSend > filelen) {
    fprintf(stderr, "UPLD: Cant send %lld byte because of file length is %lu\n", filelenToSend, filelen);
    return -3;
  }
  return 0;
}
