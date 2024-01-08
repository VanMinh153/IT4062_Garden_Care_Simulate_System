#include <stdbool.h>
#ifndef UTILITY_H
#define UTILITY_H

bool str_to_port(const char* str, int* port);
char* getTime();
int checkCommand(const char* input);

#endif // UTILITY_H
