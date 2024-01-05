#include <stdbool.h>
#ifndef UTILITY_H
#define UTILITY_H

bool str_to_port(char* str, int* port);

void clear_stdin();

char* clearstr(char* str);

#endif // UTILITY_H
