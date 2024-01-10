#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "simulator.h"

int generatedID[DEVICE_MAX];
int numID = 0;

int check_sensor_specs(int RESPONSE_TIME, int HMAX, int HMIN, int NMIN, int PMIN, int KMIN) {
  if (RESPONSE_TIME < 1 || RESPONSE_TIME > 100) return 1;
  if (HMAX < 1 || HMAX > 100) return 2;
  if (HMIN < 1 || HMIN > 100) return 3;
  if (NMIN < 1 || NMIN > 2000) return 4;
  if (PMIN < 1 || PMIN > 2000) return 5;
  if (KMIN < 1 || KMIN > 2000) return 6;
  if (HMAX < HMIN) return -1;
  return 0;
};

//----------------------------------------------------------------
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

// int get_device_info ();

// int main() {
//   for (int i = 0; i < 20; i++) {
//     printf("%d : %d\n", i + 1, genID());
//   }
//   return 0;
// }