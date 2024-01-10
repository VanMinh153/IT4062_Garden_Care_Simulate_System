#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>
#include <pthread.h>
#include <errno.h>
#include "../include/utility.h"
#include "../include/tcp_socket.h"
#include "../include/session.h"
#include "../include/simulator.h"

#define CONNECTION_MAX 1024
#define TIMEOUT 100 // 100 ms
#define TIMER_THREAD "TIMER_CONTROLLER_THREAD"
#define DATA_GET_THREAD "DATA_GET_THREAD"

char connCtrl[CONNECTION_MAX];
connection_t conninfo[CONNECTION_MAX];
int numConnect = 0; 
int end_idx = 0;
watering_t watering;
watering_t default_specs;
sensor_data_t sensor_data;
int sensorfd = -1;
char linked = 0;
char logged = 0;
char modified = 0;
char running = 0;

void* handle_thread(void* connfd);
int handle_msg(char* msg, connection_t* connection);
void get_default_specs();
void reset_default_specs();

int main(int argc, char** argv) {
  get_default_specs();
  int listenfd, connfd;
  struct sockaddr_in server, client;
  socklen_t sin_size = sizeof(struct sockaddr);
  pthread_t tid;
  int retval = -1;

  memset(connCtrl, 0, CONNECTION_MAX);
  memset(conninfo, 0, CONNECTION_MAX*sizeof(connection_t) );
  memset(&watering, 0, sizeof(watering_t) );
  memset(&default_specs, 0, sizeof(watering_t) );
  memset(&sensor_data, 0, sizeof(sensor_data_t) );
  memset(&server, 0, sizeof(server) );
  memset(&client, 0, sizeof(client) );

  if (argc != 3) {
    fprintf(stderr, "Please write: ./watering <ssid> <password>\n");
    exit(EXIT_FAILURE);
  }
  
  watering.id = genID();
  strncpy(watering.ssid, argv[1], SSID_LEN);
  strncpy(watering.password, argv[2], PASSWORD_LEN);
  strncpy(default_specs.ssid, argv[1], SSID_LEN);
  strncpy(default_specs.password, argv[2], PASSWORD_LEN);

  server.sin_family = AF_INET;
  server.sin_port = htons(SENSOR_DATA_PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  if ( (listenfd = socket(AF_INET, SOCK_STREAM, 0) ) == -1 ) {
    perror("\nsocket()");
    exit(EXIT_FAILURE);
  }
  if (bind(listenfd, (struct sockaddr*) &server, sizeof(server) ) == -1) {
    perror("\nbind()");
    exit(EXIT_FAILURE);
  }
  if (listen(listenfd, BACKLOG) == -1) {
    perror("\nlisten()");
    exit(EXIT_FAILURE);
  }
//-------------------------------------------------------------------------------  
  printf("Watering machine started!\n");
  char recv_msg[MSG_SIZE], recv_buffer[MSG_SIZE];
  char msg[MSG_SIZE];
  struct pollfd listenfd_poll;
  struct pollfd fds[CONNECTION_MAX];
  int ready = 0;
  memset(recv_msg, 0, MSG_SIZE);
  memset(recv_buffer, 0, MSG_SIZE);  
  memset(msg, 0, MSG_SIZE);
  listenfd_poll.fd = listenfd;
  listenfd_poll.events = POLLIN;
  for (int i = 0; i < CONNECTION_MAX; i++) {
    fds[i].fd = -1;
    fds[i].events = POLLIN;
  }

  gen_watering_status();
  // Create thread to control timer
  retval = pthread_create(&tid, NULL, &handle_thread, TIMER_THREAD);
  if (retval!= 0) perror("\npthread_create()");
  retval = pthread_create(&tid, NULL, &handle_thread, DATA_GET_THREAD);
  if (retval!= 0) perror("\npthread_create()");

  // Handle connection
	int id = 0;
  char ssid[SSID_LEN + 1];
  char command[COMMAND_LEN + 1];
  char type_of_msg[21];
  char overcheck = '\0';
  while (1) {
    
    if (numConnect == CONNECTION_MAX) {
      fprintf(stderr, "Error: maximum number of connections exceeded\n");
    } else {
      ready = poll(&listenfd_poll, 1, TIMEOUT);
      if (ready == -1) {
        perror("\npoll()");
        return 1;
      } else if (ready == 1) {
        connfd = accept(listenfd, (struct sockaddr*) &client, &sin_size);
        if (connfd == -1) {
          perror("\naccept()");
          return 1;
        }
        retval = get_msg(connfd, recv_msg, recv_buffer);
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
        for (int i = 0; i < CONNECTION_MAX; i++) {
          if (connCtrl[i] == 0) {
            connCtrl[i] = 1;
            fds[i].fd = connfd;
            fds[i].events = POLLIN;
            conninfo[i].id = id;
            strncpy(conninfo[i].SSID, ssid, SSID_LEN);
            conninfo[i].ip = client.sin_addr.s_addr;
            conninfo[i].port = client.sin_port;
            conninfo[i].connfd = connfd;
            conninfo[i].logged = false;
            numConnect++;
            if (i > end_idx) end_idx = i;
            break;
          }
        }
      }
    }
//----------------------------------------------------------------
// Handle message from client
    ready = poll(fds, end_idx + 1, TIMEOUT);
    if (ready == -1) {
      perror("\npoll()");
      return 1;
    } else if (ready == 0) {
      continue;
    }
    
    for (int i = 0; i <= end_idx; i++) {
      if (connCtrl[i] == 1 && fds[i].revents & POLLIN) {
        connfd = fds[i].fd;
        retval = get_msg(connfd, recv_msg, conninfo[i].recv_buffer);
        if (retval < 0) {
          if (retval == -3) {
            fprintf(stderr, "Error: messages received exceed the maximum message size\n");
            send_msg(MSG_OVERLENGTH, connfd);
          }
          close(connfd);
          connCtrl[i] = 0;
          numConnect--;
        } else {
          handle_msg(recv_msg, &conninfo[i]);
        }
      }
    }

    // optimize connCtrl array
    if (numConnect == 0) {
      end_idx = 0;
      continue;
    }

    if (connCtrl[end_idx] == 0) {
      for (int i = end_idx; i >= 0; i--) {
        if (connCtrl[i] == 1) {
          end_idx = i;
          break;
        }
      }
    }

    if (end_idx != numConnect - 1) {
      for (int i = 0; i <= end_idx; i++) {
        if (connCtrl[i] == 0) {
          conninfo[i] = conninfo[end_idx];
          fds[i].fd = fds[end_idx].fd;
          connCtrl[end_idx] = 0;
          connCtrl[i] = 1;
          for (int j = end_idx; j > i; j--) {
            if (connCtrl[j] == 1) {
              end_idx = j;
              break;
            }
          }        
        }
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
  if (strcmp(arg,DATA_GET_THREAD) == 0) {
    while (1) {
      if (linked == 1) get_sensor_data(&sensor_data, sensorfd);
      sleep(1);
    }
  } else if (strcmp(arg, TIMER_THREAD) == 0) {
    // while (1) {
    // }
    sleep(1);
  }
  pthread_exit(NULL);
}
/*
@function: handle message from client
@parameter: [IN] connection: connection's infomation
            [IN] message: message from client
@return: response message back to the client
*/
int handle_msg(char* msg, connection_t* p_conninfo) {
  connection_t conninfo1 = *p_conninfo;
  int connfd = conninfo1.connfd;
  char password[PASSWORD_LEN + 1];
  char command[COMMAND_LEN + 1];
  char type_of_msg[41];
  char overcheck = '\0';
  int retval = -1;

  memset(command, 0, COMMAND_LEN + 1);
  memset(type_of_msg, 0, 21);
  memset(password, 0, PASSWORD_LEN + 1);
  snprintf(type_of_msg, sizeof(type_of_msg), "%%%ds", COMMAND_LEN);
  sscanf(msg, type_of_msg, command);
  if (conninfo1.logged == false && strcmp(command, "LOGIN") != 0) {
    send_msg(NOT_LOGGED_IN, connfd);
    return 1;
  }
// LOGIN <password>
// > 110 // LOGIN_SUCCESS
// > 100 // LOGGED_IN
// > 201 // PASSWORD_INCORRECT
  if (strcmp(command, "LOGIN") == 0) {
    if (conninfo1.logged == true) {
      send_msg(LOGGED_IN, connfd);
      return 1;
    }
    snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %%%ds%%c", PASSWORD_LEN);
    retval = sscanf(msg, type_of_msg, password, overcheck);
    if (retval != 1) {
      send_msg(MSG_NOT_DETERMINED, connfd);
      return 1;
    }
    if (strcmp(password, watering.password) == 0) {
      p_conninfo->logged = true;
       connCtrl[p_conninfo - conninfo] = 2;
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
    if (strcmp(new_ssid, watering.ssid) == 0) {
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
    if (strcmp(password, watering.password) == 0) {
      strncpy(watering.password, new_password, PASSWORD_LEN);
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
    if (strcmp(password, watering.password) == 0) {
      reset_default_specs();
      send_msg(RESET_TO_DEFAULT, connfd);
      return 0;
    } else {
      send_msg(PASSWORD_INCORRECT, connfd);
      return 1;
    }
// //----------------------------------------------------------------
// // Watering's command
// GET
// > 150 <status> <water_mount> <HMAX> <HMIN> <list-of-timer> ... // GET_SUCCESS
  } else if (strcmp(command, "GET") == 0) {
    char watering_data[81];
    snprintf(watering_data, sizeof(watering_data), "%s %d %d %d %d", GET_SUCCESS, watering.status, watering.response_time,watering.HMAX, watering.HMIN);
    send_msg(watering_data, connfd);
    return 1;
// SET <water_mount> <HMAX> <HMIN>
// > 172 // SET_WATERING_SUCCESS
// > 203 // INVALID_ARGS
  } else if (strcmp(command, "SET") == 0) {
    int water_mount, HMAX, HMIN;
    char check = 0;
    snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %%d %%d %%d%%c");
    retval = sscanf(msg, type_of_msg, &water_mount, &HMAX, &HMIN, overcheck);
    if (retval!= 3) {
      send_msg(MSG_NOT_DETERMINED, connfd);
      return 1;
    }
    if (water_mount < 0 || water_mount > 100
        || HMAX < 1 || HMAX > 100
        || HMIN < 1 || HMIN > 100
        || HMAX < HMIN)
      check = 0;
    if (check == 0) {
      send_msg(INVALID_ARGS, connfd);
      return 1; 
    } else {
      watering.water_mount = water_mount;
      watering.HMAX = HMAX;
      watering.HMIN = HMIN;
      send_msg(SET_WATERING_SUCCESS, connfd);
      return 0;
    }
void get_default_specs() {
//****************************************************************
// future feature: get default specs from file
  default_specs.HMAX = 95;
  default_specs.HMIN = 45;
}
void gen_watering_status() {
  watering.HMAX = default_specs.HMAX;
  watering.HMIN = default_specs.HMIN;
}
void reset_default_specs() {
  strcpy(watering.ssid, default_specs.ssid);
  strcpy(watering.password, default_specs.password);
  watering.response_time = default_specs.response_time;
}
