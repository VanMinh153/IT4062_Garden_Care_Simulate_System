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
  device_t deviceA[16][128];
  device_t deviceB[2048];
  garden_t gardenA[16];
  int ndeviceA = 0;

  memset(deviceA, 0, sizeof(deviceA) );
  memset(deviceB, 0, sizeof(deviceB) );
  memset(gardenA, 0, sizeof(gardenA) );

  strncpy(gardenA[0].name, "Potato Area", NAME_SIZE);
  gardenA[0].ndevice = 4;
  deviceA[0][0].id = 1994;
  deviceA[0][0].type = 1;
  deviceA[0][1].id = 1989;
  deviceA[0][1].type = 2;
  deviceA[0][2].id = 1975;
  deviceA[0][2].type = 3;
  deviceA[0][3].id = 1972;
  deviceA[0][3].type = 4;

  strncpy(gardenA[1].name, "Tomato Area", NAME_SIZE);
  gardenA[1].ndevice = 4;
  deviceA[1][0].id = 2994;
  deviceA[1][0].type = 1;
  deviceA[1][1].id = 2989;
  deviceA[1][1].type = 2;
  deviceA[1][2].id = 2975;
  deviceA[1][2].type = 3;
  deviceA[1][3].id = 2972;
  deviceA[1][3].type = 4;

//  printf("-------------Automatic garden care system-------------\n");
  printf("_____________________________________________________\n");
  printf("|                                                    |\n");
  printf("|           AUTOMATIC GARDEN CARE SYSTEM             |\n");
  printf("|____________________________________________________|\n\n");
  printf("List of devices\n");

  for (int i = 0; i < 16; i++) {
    int ndevice = gardenA[i].ndevice;
    if (ndevice == 0) continue;
    printf("\n  %2d. %s\n", i+1, gardenA[i].name);
    printf("_____________________________\n");
    printf("|__id__|________name_________|\n");
//    printf("|  id  |        name         |\n");

    int x = i, y = 0;
    while (1) {
      int id = deviceA[x][y].id;
      int type = deviceA[x][y].type;
      switch (type) {
//               printf("|  id  |        name         |n");
        case 1: printf("| %4d |   Sensor            |\n", id); break;
        case 2: printf("| %4d |   Watering machine  |\n", id); break;
        case 3: printf("| %4d |   NPK fertilization |\n", id); break;
        case 4: printf("| %4d |   Lamp              |\n", id); break;
        default: fprintf(stderr, "Error: Invalid device\n"); break;
      }
      y++;
      ndevice--;
      if (ndevice == 0) {
        printf("--- End\n");
//        printf("------------------------------\n");
//        printf("|______|_____________________|\n");
        break;
      }
    }
  }
  return 0;
}
