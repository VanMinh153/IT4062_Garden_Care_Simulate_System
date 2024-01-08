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