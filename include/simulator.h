#ifndef SIMULATOR_H
#define SIMULATOR_H

#define DEVICE_MAX 100
#define SSID_LEN 20
#define PASSWORD_LEN 10
#define DEFAULT_RESPONSE_TIME 100 // 100 ms
// Type of device
#define SENSOR 1
#define WATERING 2
#define FERTILIZING 3
#define LAMP 4
#define DEFAULT_SPECS_SRC "data/default-specs.txt"

// Environmental sensors measure humidity and concentrations of nutrients nitrogen (N), phosphorus (P), and potassium (K) in the soil. The sensors record data every T minutes.
struct sensor_t {
  int id;
  char ssid[SSID_LEN + 1];
  char password[PASSWORD_LEN + 1];
  char status;
// 0: default
// 1: modified
  unsigned int RESPONSE_TIME; // 1-100 second
  int humidity; // 0-100%
  int HMAX;
  int HMIN;
  unsigned int nitrogen;    // 0-2000 mg/kg
  unsigned int NMIN;
  unsigned int phosphorus;  // 0-2000 mg/kg
  unsigned int PMIN;
  unsigned int potassium;   // 0-2000 mg/kg
  unsigned int KMIN;
};
typedef struct sensor_t sensor_t;

// The automatic watering unit irrigates simultaneously with 2 mechanisms: automatic watering at fixed times of the day set by the user if the soil moisture does not exceed the Hmax threshold and automatic watering if the soil moisture is below Hmin threshold. The Hmax and Hmin thresholds are set by the user.
struct watering_t {
  int id;
  char ssid[SSID_LEN + 1];
  char password[PASSWORD_LEN + 1];
  char status;
// perform as bits
// 0000: not linked, timer off, default, stopped
// 1111: linked, timer on, modified, running
  unsigned int water_mount;  // liter
  char timer[10][4];
// timer[i] = "abcd" : "ab" is hour, "cd" is minute
  int sensor_id;
  int connfd;
};
typedef struct watering_t watering_t;

// Automatic NPK fertilization unit: when the concentration of nutrients is below the threshold of Nmin, Pmin, Kmin, this system automatically adds an amount of fertilizer mixed into the water at a concentration of C grams/liter and sprinkles an amount of V liters. Parameters set by the user.
struct fertilizing_t {
  int id;
  char ssid[SSID_LEN + 1];
  char password[PASSWORD_LEN + 1];
  char status;
// perform as bits
// 0000: not linked, timer off, default, stopped
// 1111: linked, timer on, modified, running
  unsigned int N_mount; // 0-1000 gram/lit
  unsigned int P_mount; // 0-1000 gram/lit
  unsigned int K_mount; // 0-1000 gram/lit
  unsigned int water_mount;  // 0-100 liter
  char timer[10][4];
// timer[i] = "abcd" : "ab" is hour, "cd" is minute
  int sensor_id;
  int connfd;
};
typedef struct fertilizing_t fertilizing_t;

// Photosynthesis support lamp part: glows with power P according to the time scheduled by the user.
struct lamp_t {
  int id;
  char ssid[SSID_LEN + 1];
  char password[PASSWORD_LEN + 1];
  char status;
// perform as bits
// 000: timer off, default, stopped
// 111: timer on, modified, running
  unsigned int power; // 5-200 watt
  unsigned int time; // 1-1000 minute, -1 is infinite
  char timer[10][11];
// timer[i] = "abcdefghijk"
// "ab":"cd" is start time
// "ef":"gh" is end time
// "ijk" is power, 5-200 watt, "000" is current power setted
};
typedef struct lamp_t lamp_t;

//----------------------------------------------
int genID();
int get_default_specs(void* dest, int type);
int reset_default_specs(void* dest, void* default_specs, int type);

//----------------------------------------------
// Default specs struct

#endif // SIMULATOR_H
