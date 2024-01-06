#ifndef SIMULATOR_H
#define SIMULATOR_H

#define DEVICE_MAX 100
#define SSID_LEN 20
#define PASSWORD_LEN 10
#define DEFAULT_RESPONSE_TIME 100 // 100 ms

// Environmental sensors measure humidity and concentrations of nutrients nitrogen (N), phosphorus (P), and potassium (K) in the soil. The sensors record data every T minutes.
struct sensor_t {
  unsigned int id;
  char ssid[SSID_LEN + 1];
  char password[PASSWORD_LEN + 1];
  unsigned int RESPONSE_TIME; // = DEFAULT_RESPONSE_TIME;
  int humidity;  // 0-100%
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
struct watering_machine_t {
  unsigned int id;
  char ssid[SSID_LEN + 1];
  char password[PASSWORD_LEN + 1];
  char status;   // 1 if active, 0 if inactive
  unsigned int water_mount;  // liter
  time_t time_start;
  time_t time_end;
};
typedef struct watering_machine_t watering_machine_t;

// Automatic NPK fertilization unit: when the concentration of nutrients is below the threshold of Nmin, Pmin, Kmin, this system automatically adds an amount of fertilizer mixed into the water at a concentration of C grams/liter and sprinkles an amount of V liters. Parameters set by the user.
struct fertilizing_machine_t {
  unsigned int id;
  char ssid[SSID_LEN + 1];
  char password[PASSWORD_LEN + 1];
  char status;      // 1 if active, 0 if inactive
  unsigned int N_mount; // gram/lit
  unsigned int P_mount; // gram/lit
  unsigned int K_mount; // gram/lit
  unsigned int water_mount;  // liter
};
typedef struct fertilizing_machine_t fertilizing_machine_t;

// Photosynthesis support lamp part: glows with power P according to the time scheduled by the user.
struct lamp {
  unsigned int id;
  char ssid[SSID_LEN + 1];
  char password[PASSWORD_LEN + 1];
  char status;  // 1 if active, 0 if inactive
  unsigned int power;    // watts
  time_t time_start;
  time_t time_end;
};
typedef struct lamp lamp;

int genID();

#endif // SIMULATOR_H
