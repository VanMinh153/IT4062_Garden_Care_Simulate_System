#include <stdbool.h>
#ifndef HW5_UTILITY_H
#define HW5_UTILITY_H

extern char strTime[22];

bool str_to_port(char* str, int* port);

char* get_time();

void clear_stdin();

#endif // HW5_UTILITY_H
