#include <stdbool.h>
#include "simulator.h"
#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#define CONNECT_MSG "IT4062"
#define BACKLOG 20
#define MSG_SIZE 2048   // Size of string message
#define DELIMITER "\r\n"
#define DELIMITER_LEN 2
#define DELIMITER_SIZE 3
#define COMMAND_LEN 16    // Max length of command
#define CTRL_LOOP 5
//----------------------------------------------------------------
// Protocol design
#define LOGGED_IN "100"
#define SENSOR_CONNECTED "101"
#define WATERING_CONNECTED "102"
#define FERTILIZING_CONNECTED "103"
#define LAMP_CONNECTED "104"
#define CLIENT_CONNECTED "105"

#define LOGIN_SUCCESS "110"
#define SSID_CHANGE_SUCCESS "120"
#define PASSWORD_CHANGE_SUCCESS "121"
#define LINK_SUCCESS "130"
#define RUN_SUCCESS "140"
#define STOP_SUCCESS "141"
#define GET_SUCCESS "150"
#define ADD_TIMER_SUCCESS "160"
#define ALREADY_EXIST "160"
#define REMOVE_TIMER_SUCCESS "162"
#define SET_SENSOR_SUCCESS "171"
#define SET_WATERING_SUCCESS "172"
#define SET_FERTILIZING_SUCCESS "173"
#define SET_LAMP_SUCCESS "174"
#define TIMER_ON_SUCCESS "180"
#define TIMER_OFF_SUCCESS "181"
#define RESET_TO_DEFAULT "190"

#define NOT_LOGGED_IN "200"
#define PASSWORD_INCORRECT "201"
#define NOT_LINKED "202"
#define INVALID_ARGS "203"
#define INVALID_SSID "220"
#define INVALID_PASSWORD "221"
#define LINK_FAILED "230"
#define INVALID_TIME "260"
#define MAXIMUM_TIMER "261"
#define NOT_FOUND "262"

#define MSG_OVERLENGTH "300"
#define UNKNOWN_COMMAND "301"
#define MSG_NOT_DETERMINED "302"

#define UPDATE_SUCCESS "400"
#define WRITE_SUCCESS "401"
//----------------------------------------------------------------
// response message
#define M100 "Have been logged in"
#define M101 "Sensor connected"
#define M102 "Watering connected"
#define M103 "Fertilizing connected"
#define M104 "Lamp connected"
#define M105 "Client connected"

#define M110 "Login successful"
#define M120 "SSID changed"
#define M121 "Password changed"
#define M130 "Link successful"
#define M140 "Run successful"
#define M141 "Stop successful"
#define M150 "Get successful"
#define M160 "Add timer successful"
#define M161 "Already exist"
#define M162 "Remove timer successful"
#define M171 "Set sensor successful"
#define M172 "Set watering successful"
#define M173 "Set fertilizing successful"
#define M174 "Set lamp successful"
#define M180 "Timer on successful"
#define M181 "Timer off successful"
#define M190 "Reset to default successful"

#define M200 "Not logged in"
#define M201 "Password incorrect"
#define M202 "Not linked"
#define M203 "Invalid arguments"
#define M220 "Invalid SSID"
#define M221 "Invalid password"
#define M230 "Link failed"
#define M260 "Invalid time"
#define M261 "Maximum timer"
#define M262 "Not found"

#define M300 "Message overlength"
#define M301 "Unknown command"
#define M302 "Message not determined"

#define M400 "Update successful"
#define M401 "Write successful"
//----------------------------------------------------------------

struct connection_t {
  int id;
  char SSID[SSID_LEN];
  unsigned int ip;
  unsigned int port;
  int connfd;
  int connCtrl_idx;
  bool hasLogin;
};
typedef struct connection_t connection_t;
bool send_msg(char* msg, int connfd);

int get_msg(int connfd, char* message, char* recv_buffer, int timeout);

char* str_to_msg(char* msg_code);

bool print_msg(char* msg_code);

int echo_server(int connfd);

int echo_client(int connfd);

void *echo_thread(void* connfd);

#endif // TCP_SOCKET_H