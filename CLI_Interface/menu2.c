#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NAME_SIZE 128

typedef struct device_t {
  unsigned int id;
  unsigned char type;
} device_t;

typedef struct garden_t {
  char name[NAME_SIZE];
  unsigned int ndevice;
  int nito;     // nitrogen
  int kali;     // potassium
  int photpho;  // phosphorus
  float humid;
} garden_t;

int main() {
  printf("_____________________________________________________\n");
  printf("|                                                    |\n");
  printf("|           AUTOMATIC GARDEN CARE SYSTEM             |\n");
  printf("|____________________________________________________|\n\n");
  printf("1. Scan all device\n");
  printf("2. See garden device data\n");
  printf("3. See garden sensor data\n");
  printf("4. Connect to one device\n");
  printf("5. Exit");
  printf("\n\n");
  return 0;
}

