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
#define COMMAND_SIZE 10 // Size of string command

#define CLIENT_CONNECTED "100"
#define SENSOR_CONNECTED "101"
#define WATERING_MACHINE_CONNECTED "102"
#define FERTILIZING_MACHINE_CONNECTED "103"
#define LAMP_CONNECTED "104"
// #define LOGIN_SUCCESS "110"
// #define ACCOUNT_LOCKED "211"
// #define ACCOUNT_NOT_EXIST "212"
// #define LOGGED_IN "213"
// #define NOT_LOGGED_IN "221"
// #define POST_SUCCESS "120"
// #define LOGOUT_SUCCESS "130"
#define MSG_NOT_DETERMINED "300"
#define MSG_OVERLENGTH "301"

#define M100 "Connect successful"
// #define M110 "Login successful"
// #define M211 "Account is locked"
// #define M212 "Account does not exist"
// #define M213 "Have been logged in"
// #define M221 "Have not logged in"
// #define M120 "Post successful"
// #define M130 "Logout successful"
#define M300 "Message cannot determined"
#define M301 "Message exceed the maximum message size"

struct connection_t {
  int id;
  char SSID[SSID_LEN];
  unsigned int ip;
  unsigned short int port;
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