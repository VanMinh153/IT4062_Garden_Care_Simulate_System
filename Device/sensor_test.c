#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
//#include <sys/wait.h>
#include <pthread.h>
#include <errno.h>
#include "../include/utility.h"
#include "../include/tcp_socket.h"
#include "../include/session.h"
#include "../include/simulator.h"

#define CONNECTION_MAX 1024
#define TIMEOUT 3
#define ARG_THREAD_2 "sensor_simulator"
#define PERCENT_CHANGE 99 // 99%

int PORT = 0;
char connCtrl[CONNECTION_MAX];
connection_t conninfo[CONNECTION_MAX];
unsigned int numConnect = 0; 
sensor_t sensor;

void* handle_thread(void* connfd);
int handle_msg(char* msg, connection_t* connection);
void gen_sensor_status();
void ctrl_sensor_status();

int main(int argc, char** argv) {
  int listenfd, connfd;
  struct sockaddr_in server, client;
  char clientIP[INET_ADDRSTRLEN];
  socklen_t sin_size = sizeof(struct sockaddr);
  pthread_t tid;
  char recv_msg[MSG_SIZE], recv_buffer[MSG_SIZE];
  int retval = -1;

  memset(connCtrl, 0, CONNECTION_MAX);
  memset(conninfo, 0, CONNECTION_MAX*sizeof(connection_t) );
  memset(&sensor, 0, sizeof(sensor_t) );
  memset(&server, 0, sizeof(server) );
  memset(recv_msg, 0, MSG_SIZE);
  memset(recv_buffer, 0, MSG_SIZE);

  if (argc != 3) {
    fprintf(stderr, "Please write: ./sensor <ssid> <password> <port>\n");
    exit(EXIT_FAILURE);
  }
  if (!str_to_port(argv[1], &PORT)) {
    fprintf(stderr, "Error: Port number is invalid for user server applications\n");
    exit(EXIT_FAILURE);
  }

  sensor.id = genID();
  strncpy(sensor.ssid, argv[1], SSID_LEN);
  strncpy(sensor.password, argv[2], PASSWORD_LEN);  

  if ( (listenfd = socket(AF_INET, SOCK_STREAM, 0) ) == -1 ) {
    perror("\nsocket()");
    exit(EXIT_FAILURE);
  }

  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(listenfd, (struct sockaddr*) &server, sizeof(server) ) == -1) {
    perror("\nbind()");
    exit(EXIT_FAILURE);
  }

  if (listen(listenfd, BACKLOG) == -1) {
    perror("\nlisten()");
    exit(EXIT_FAILURE);
  }
  
  printf("Sensor started!\n");
  gen_sensor_status();
  // thread to control sensor status
  retval = pthread_create(&tid, NULL, &handle_thread, ARG_THREAD_2);
  if (retval != 0) perror("\npthread_create()");

  // Handle connection
	while (1) {
    if (numConnect >= CONNECTION_MAX) {
      fprintf(stderr, "Error: maximum number of connections exceeded\n");
      sleep(3);
      continue;
    }
    if ( (connfd = accept(listenfd, (struct sockaddr*) &client, &sin_size) ) == -1) {
      perror("\naccept()");
      return 1;
    }
    
    retval = get_msg(connfd, recv_msg, recv_buffer, TIMEOUT);
    if (strcmp(recv_msg, CONNECT_MSG) != 0) {
      fprintf(stderr, "Error: connect message is not received\n");
      close(connfd);
      continue;
    }

    inet_ntop(AF_INET, &client.sin_addr, clientIP, sizeof(clientIP) );
    // Can print client's IP and client's port here
    for (int i = 0; i < CONNECTION_MAX; i++) {
      if (connCtrl[i] == 0) {
        connCtrl[i] = 1;
        conninfo[i].ip = client.sin_addr.s_addr;
        conninfo[i].port = client.sin_port;
        conninfo[i].connfd = connfd;
        conninfo[i].connCtrl_idx = i;
        conninfo[i].hasLogin = false;
        // numConnect++;
        retval = pthread_create(&tid, NULL, &handle_thread, &conninfo[i]);
        if (retval != 0) perror("\npthread_create()");
        // printf("Number of connection: %d\n", numConnect);
        break;
      }
    }
  }
	close(listenfd);
	return 0;
}
/*
@function: Get and handle message from client in a new thread
@parameter: [IN] arg: connection's infomation
@return: response message back to the client
*/
void* handle_thread(void* arg) {
  pthread_detach(pthread_self());
  if (strcmp((char*) arg, ARG_THREAD_2) == 0) {
    while (1) {
      ctrl_sensor_status();
      sleep(3);
    }
  }

  connection_t* p_conninfo = (connection_t*) arg;
  connection_t conninfo = *( (connection_t*) arg);
  int connfd = conninfo.connfd;
  char recv_msg[MSG_SIZE], recv_buffer[MSG_SIZE];
  char msg[MSG_SIZE];
  memset(recv_msg, 0, MSG_SIZE);
  memset(recv_buffer, 0, MSG_SIZE);
  memset(msg, 0, MSG_SIZE);
  int retval = -1;

  snprintf(msg, MSG_SIZE, "%s %d %s", SENSOR_CONNECTED, sensor.id, sensor.ssid);
  send_msg(msg, connfd);
  
  while (1) {
    retval = get_msg(connfd, recv_msg, recv_buffer);
    if (retval < 0) {
      if (retval == -3) {
        fprintf(stderr, "Error: messages received exceed the maximum message size\n");
        fprintf(stderr, "Notice: message length is limited to %d characters\n", MSG_SIZE);
        send_msg(MSG_NOT_DETERMINED, connfd);
      } else {
        close(connfd);
        connCtrl[conninfo.connCtrl_idx] = 0;
        pthread_exit(NULL);
      }
    }
    handle_msg(recv_msg, p_conninfo);
  }
}
/*
@function: handle message from client
@parameter: [IN] connection: connection's infomation
            [IN] message: message from client
@return: response message back to the client
*/
int handle_msg(char* msg, connection_t* p_conninfo) {
  connection_t conninfo = *p_conninfo;
  bool hasLogin = conninfo.hasLogin;
  int connfd = conninfo.connfd;
  char command[COMMAND_SIZE];
  char type_of_msg[21];
  char overcheck = '\0';
  int retval = -1;
  sscanf(msg, "%4s", command); // Get 4 character for the command
  // handle the LOGIN command
  if (strcmp(command, "USER") == 0) {
    snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %%%ds %%c", NAME_SIZE);
    retval = sscanf(msg, type_of_msg, username, overcheck);

    if (strlen(username) == NAME_SIZE) {
      send_msg(ACCOUNT_NOT_EXIST, connfd);
      return 0;
    }
    if ( retval < 1 || overcheck != '\0' ) {
      send_msg(ACCOUNT_NOT_EXIST, connfd);
      return 0;
    }
    if (hasLogin == true) {
      send_msg(LOGGED_IN, connfd);
      return 0;
    }

    bool accExitst = false;
    for (int i = 0; i < numAccount; i++) {
      if (strcmp(username, accounts[i].username) == 0) {
        accExitst = true;
        if (accounts[i].status == false) {
          send_msg(ACCOUNT_LOCKED, connfd);
          return 0;
        }
      }
    }
    if (accExitst == false) {
      send_msg(ACCOUNT_NOT_EXIST, connfd);
      return 0;
    }

    session->hasLogin = true;
    memmove(session->username, username, sizeof(username));
    // Can print user logged in successful here
    send_msg(LOGIN_SUCCESS, connfd);
    return 0;
  
  // handle the POST command
  } else if (strcmp(command, "POST") == 0) {
    if (hasLogin == false) {
      send_msg(NOT_LOGGED_IN, connfd);
      return 0;
    }
    // Can print post content here
    send_msg(POST_SUCCESS, connfd);
    return 0;
  // handle the BYE command
  } else if (strcmp(command, "BYE") == 0) {
    if (hasLogin == false) {
      send_msg(NOT_LOGGED_IN, connfd);
      return 0;
    }
    session->hasLogin = false;
    *session->username = '\0';
    send_msg(LOGOUT_SUCCESS, connfd);
    return 0;
  } else {
    send_msg(MSG_NOT_DETERMINED, connfd);
    return 0;
  }
  return 0;
}
void gen_sensor_status() {
  sensor.humidity = rand() % 51 + 50;
  sensor.nitrogen = rand() % 1001 + 1000;
  sensor.phosphorus = rand() % 1001 + 1000;
  sensor.potassium = rand() % 1001 + 1000;
}
void ctrl_sensor_status() {
  sensor.humidity += rand() % 21 - 10;
  if (sensor.nitrogen + sensor.phosphorus + sensor.potassium < 500) return;
  sensor.nitrogen *= PERCENT_CHANGE/100;
  sensor.phosphorus *= PERCENT_CHANGE/100;
  sensor.potassium *= PERCENT_CHANGE/100;
}