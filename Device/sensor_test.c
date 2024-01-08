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
sensor_t default_specs;

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
  memset(&default_specs, 0, sizeof(sensor_t) );
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
    unsigned int id;
    char ssid[SSID_LEN + 1];
    char command[COMMAND_LEN + 1];
    char type_of_msg[21];
    char overcheck = '\0';

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
    snprintf(type_of_msg, sizeof(type_of_msg), "%%%ds %%d %%%ds%%c", COMMAND_LEN, SSID_LEN);
    retval = sscanf(recv_msg, type_of_msg, command, id, ssid, overcheck);
    if (retval != 3) {
      send_msg(MSG_NOT_DETERMINED, connfd);
      return 1;
    }

    if (strcmp(command, "CONNECT") != 0) {
      close(connfd);
      continue;
    }
    
    inet_ntop(AF_INET, &client.sin_addr, clientIP, sizeof(clientIP) );
    // Can print client's IP and client's port here
    for (int i = 0; i < CONNECTION_MAX; i++) {
      if (connCtrl[i] == 0) {
        connCtrl[i] = 1;
        conninfo[i].id = id;
        strncpy(conninfo[i].SSID, ssid, SSID_LEN);
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
    retval = get_msg(connfd, recv_msg, recv_buffer, 0);
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
  int connfd = conninfo.connfd;
  char password[PASSWORD_LEN + 1];
  char command[COMMAND_LEN + 1];
  char type_of_msg[21];
  char overcheck = '\0';
  int retval = -1;

  memset(command, 0, COMMAND_LEN + 1);
  memset(type_of_msg, 0, 21);
  memset(password, 0, PASSWORD_LEN + 1);
  snprintf(type_of_msg, sizeof(type_of_msg), "%%%ds", COMMAND_LEN);
  sscanf(msg, type_of_msg, command);
  if (conninfo.hasLogin == false && strcmp(command, "LOGIN") != 0) {
    send_msg(NOT_LOGGED_IN, connfd);
    return 1;
  }
// LOGIN <password>
// > 110 // LOGIN_SUCCESS
// > 100 // LOGGED_IN
// > 201 // PASSWORD_INCORRECT
  if (strcmp(command, "LOGIN") == 0) {
    if (conninfo.hasLogin == true) {
      send_msg(LOGGED_IN, connfd);
      return 1;
    }
    snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %%%ds%%c", PASSWORD_LEN);
    retval = sscanf(msg, type_of_msg, password, overcheck);
    if (retval != 1) {
      send_msg(MSG_NOT_DETERMINED, connfd);
      return 1;
    }
    if (strcmp(password, sensor.password) == 0) {
      p_conninfo->hasLogin = true;
      send_msg(LOGIN_SUCCESS, connfd);
      return 0;
    } else {
      send_msg(PASSWORD_INCORRECT, connfd);
      return 1;
    }
// SSID <new_ssid>
// > 120 // SSID_CHANGE_SUCCESS
// > 220 // INVALID_SSID
  } else if (strcmp(command, "SSID") == 0) {
    char new_ssid[SSID_LEN + 1];
    snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %%%ds%%c", SSID_LEN);
    retval = sscanf(msg, type_of_msg, new_ssid, overcheck);
    if (retval != 1) {
      send_msg(MSG_NOT_DETERMINED, connfd);
      return 1;
    }
    if (strcmp(new_ssid, sensor.ssid) == 0) {
      send_msg(SSID_CHANGE_SUCCESS, connfd);
      return 0;
    } else {
      send_msg(INVALID_SSID, connfd);
      return 1;
    }
// PASSWD <old_password> <new_password>
// > 121 // PASSWORD_CHANGE_SUCCESS
// > 221 // INVALID_PASSWORD
// > 201 // PASSWORD_INCORRECT
  } else if (strcmp(command, "PASSWD") == 0) {
    char new_password[PASSWORD_LEN + 1];
    snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %%%ds %%%ds%%c", PASSWORD_LEN, PASSWORD_LEN);
    retval = sscanf(msg, type_of_msg, password, new_password, overcheck);
    if (retval != 2) {
      send_msg(MSG_NOT_DETERMINED, connfd);
      return 1;
    }
    if (strcmp(password, sensor.password) == 0) {
      strncpy(sensor.password, new_password, PASSWORD_LEN);
      send_msg(PASSWORD_CHANGE_SUCCESS, connfd);
      return 0;
    } else {
      send_msg(PASSWORD_INCORRECT, connfd);
      return 1;
    }
// RESET <password>
// > 190 // RESET_TO_DEFAULT
// > 201 // PASSWORD_INCORRECT
  } else if (strcmp(command, "RESET") == 0) {
    snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %%%ds%%c", PASSWORD_LEN);
    retval = sscanf(msg, type_of_msg, password, overcheck);
    if (retval!= 1) {
      send_msg(MSG_NOT_DETERMINED, connfd);
      return 1;
    }
    if (strcmp(password, sensor.password) == 0) {
      reset_default_specs();
      send_msg(RESET_TO_DEFAULT, connfd);
      return 0;
    } else {
      send_msg(PASSWORD_INCORRECT, connfd);
      return 1;
    }
// //----------------------------------------------------------------
// // Sensor's command
// GET
// > 150 <status> <humidity> <nitrogen> <phosphorus> <potassium> <RESPONSE_TIME> <HMAX> <HMIN> <NMIN> <PMIN> <KMIN> // GET_SUCCESS
  } else if (strcmp(command, "GET") == 0) {
    char status[STATUS_LEN + 1];
    snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %%%ds%%c", STATUS_LEN);
    retval = sscanf(msg, type_of_msg, status, overcheck);
    if (retval != 1) {
      send_msg(MSG_NOT_DETERMINED, connfd);
      return 1;
    }
    if (strcmp(status, "SENSOR") == 0) {
      snprintf(msg, MSG_SIZE, "%s %s %d %d %d %d %d %d %d %d %d", GET_SUCCESS, status, sensor.humidity, sensor.nitrogen, sensor.phosphorus, sensor.potassium, sensor.response_time, sensor.hmax, sensor.hmin, sensor.nmin, sensor.pmin, sensor.kmin);
      send_msg(msg, connfd);
      return 0;
    } else {
      send_msg(INVALID_ARGS, connfd);
      return 1;
    }
  } else if (strcmp(command, "SET") == 0) {
    int response_time, hmax, hmin, nmin, pmin, kmin;
    snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %d %d %d %d %d %d%%c", &response_time, &hmax, &hmin, &nmin, &pmin, &kmin, overcheck);
    retval = sscanf(msg, type_of_msg);
    if (retval != 6) {
      send_msg(MSG_NOT_DETERMINED, connfd);
      return 1;
    }
    if (response_time < 0 || hmax < 0 || hmin < 0 || nmin < 0 || pmin < 0 || kmin < 0) {
      send_msg(INVALID_ARGS, connfd);
      return 1;
    }
    sensor.response_time = response_time;
    sensor.hmax = hmax;
    sensor.hmin = hmin;
    sensor.nmin = nmin;
    sensor.pmin = pmin;
    sensor.kmin = kmin;
    send_msg(SET_SENSOR_SUCCESS, connfd);
    return 0;
// SET <RESPONSE_TIME> <HMAX> <HMIN> <NMIN> <PMIN> <KMIN>
// > 171 // SET_SENSOR_SUCCESS
// > 203 // INVALID_ARGS
// // simulation command
// UPDATE HUMID
// > 400 // UPDATE_SUCCESS
// // setup
// UPDATE NPK
// > 400 // UPDATE_SUCCESS
// // humidity += (100-humidity)*0.5;
// WRITE <humidity> <nitrogen> <phosphorus> <potassium>
// > 401 // WRITE_SUCCESS

}
// int handle_connect(int connfd, int* p_id, char* p_ssid) {
//   unsigned int id;
//   char ssid[SSID_LEN + 1];
//   char command[COMMAND_LEN + 1];
//   char type_of_msg[21];
//   char overcheck = '\0';
//   int retval = -1;

//   retval = get_msg(connfd, recv_msg, recv_buffer, TIMEOUT);
//   snprintf(type_of_msg, sizeof(type_of_msg), "%%%ds %%d %%%ds%%c", COMMAND_LEN, SSID_LEN);
//   retval = sscanf(recv_msg, type_of_msg, command, id, ssid, overcheck);
//   if (retval != 3) {
//     send_msg(MSG_NOT_DETERMINED, connfd);
//     return 1;
//   }

//   if (strcmp(command, "CONNECT") != 0) {
//     close(connfd);
//     return 1;
//   }
//   id = 
// }
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