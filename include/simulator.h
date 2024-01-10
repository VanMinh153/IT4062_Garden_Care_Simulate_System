#ifndef SIMULATOR_H
#define SIMULATOR_H

#define DEVICE_MAX 100
#define SSID_LEN 20
#define PASSWORD_LEN 10
#define TIMER_COUNT 10
// Type of device
#define SENSOR 1
#define WATERING 2
#define FERTILIZING 3
#define LAMP 4
//
#define DEFAULT_RESPONSE_TIME 100 // 100 ms
#define SENSOR_STATUS_BIT_COUNT 1
#define WATERING_STATUS_BIT_COUNT 4
#define FERTILIZING_STATUS_BIT_COUNT 4
#define LAMPING_STATUS_BIT_COUNT 3
// Source path
#define DEFAULT_SPECS_SRC "data/default-specs.txt"
//
struct sensor_data_t {
  int humidity; // 0-100%
  int nitrogen;     // 0-2000 mg/kg
  int phosphorus;   // 0-2000 mg/kg
  int potassium;    // 0-2000 mg/kg
};
typedef struct sensor_data_t sensor_data_t;
// Environmental sensors measure humidity and concentrations of nutrients nitrogen (N), phosphorus (P), and potassium (K) in the soil. The sensors record data every T minutes.
struct sensor_t {
  int id;
  char ssid[SSID_LEN + 1];
  char password[PASSWORD_LEN + 1];
  char status;
// 0: default
// 1: modified
  int response_time; // 1-100 second
  int humidity; // 0-100%
  int nitrogen;    // 0-2000 mg/kg
  int phosphorus;  // 0-2000 mg/kg
  int potassium;   // 0-2000 mg/kg
};
typedef struct sensor_t sensor_t;

// The automatic watering unit irrigates simultaneously with 2 mechanisms: automatic watering at fixed times of the day set by the user if the soil moisture does not exceed the Hmax threshold and automatic watering if the soil moisture is below Hmin threshold. The Hmax and Hmin thresholds are set by the user.
struct watering_t {
  int id;
  char ssid[SSID_LEN + 1];
  char password[PASSWORD_LEN + 1];
  int HMAX;
  int HMIN;
  char status;
// perform as bits
// 0000: not linked, timer off, default, stopped
// 1111: linked, timer on, modified, running
  int water_mount;  // liter
  char timer[TIMER_COUNT][4];
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
  int NMIN; // 0-2000 mg/kg
  int PMIN; // 0-2000 mg/kg
  int KMIN; // 0-2000 mg/kg
  int N_mount; // 0-1000 gram/lit
  int P_mount; // 0-1000 gram/lit
  int K_mount; // 0-1000 gram/lit
  int water_mount;  // 0-100 liter
  char timer[TIMER_COUNT][4];
// timer[i] = "abcd" : "ab" is hour, "cd" is minute
  int sensor_id;
  int connfd;
};
typedef struct fertilizing_t fertilizing_t;

struct lamp_timer_t {
  char time_start[4];
// timer[i] = "abcd" : "ab" is hour, "cd" is minute
  char time_end[4];
  int power; // 0 is setting to use current power
};

// Photosynthesis support lamp part: glows with power P according to the time scheduled by the user.
struct lamp_t {
  int id;
  char ssid[SSID_LEN + 1];
  char password[PASSWORD_LEN + 1];
  char status;
// perform as bits
// 000: timer off, default, stopped
// 111: timer on, modified, running
  int power; // 5-200 watt
  int time; // 1-1000 minute, -1 is infinite
  struct lamp_timer_t timer[TIMER_COUNT];
};
typedef struct lamp_t lamp_t;

//----------------------------------------------
int genID();
int check_sensor_specs(int RESPONSE_TIME, int HMAX, int HMIN, int NMIN, int PMIN, int KMIN);

//----------------------------------------------

#endif // SIMULATOR_H
