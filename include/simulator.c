#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "simulator.h"

int generatedID[DEVICE_MAX];
int numID = 0;

int genID() {
  int id;
  int check = 1;
  // srand(time(NULL)); // seed the random number generator with the current time
  while (1) {
    id = rand() % (9000) + 1000;
    for (int i = 0; i < numID; i++) {
      if (id == generatedID[i]) {
        check = 0;
        break;
      }
    }
    if (check == 1) break;
  }
  return id;
}

int get_device_info ()

int main() {
  for (int i = 0; i < 20; i++) {
    printf("%d : %d\n", i + 1, genID());
  }
  return 0;
}