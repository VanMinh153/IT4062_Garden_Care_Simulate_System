#include <stdbool.h>
#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#define BACKLOG 20
#define MSG_SIZE 2048   // Size of string message
#define DELIMITER "\r\n"
#define DELIMITER_LEN 2
#define DELIMITER_SIZE 3

#define CONNECT_SUCCESS "100"
#define LOGIN_SUCCESS "110"
#define ACCOUNT_LOCKED "211"
#define ACCOUNT_NOT_EXIST "212"
#define LOGGED_IN "213"
#define NOT_LOGGED_IN "221"
#define POST_SUCCESS "120"
#define LOGOUT_SUCCESS "130"
#define MSG_NOT_DETERMINED "300"
#define MSG_OVERLENGTH "301"

#define M100 "Connect successful"
#define M110 "Login successful"
#define M211 "Account is locked"
#define M212 "Account does not exist"
#define M213 "Have been logged in"
#define M221 "Have not logged in"
#define M120 "Post successful"
#define M130 "Logout successful"
#define M300 "Message cannot determined"
#define M301 "Message exceed the maximum message size"


bool send_msg(char* msg, int connfd);

char* strmsg(char* msg_code);

bool print_msg(char* msg_code);

int get_msg(int clientfd, char* message, char* recv_buffer);

#endif // TCP_SOCKET_H

