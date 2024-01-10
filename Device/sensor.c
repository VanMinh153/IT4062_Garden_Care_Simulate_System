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
#define CONTROLLER_THREAD "SENSOR_CONTROLLER_THREAD"
#define DATA_SEND_THREAD "SENSOR_DATA_SEND_THREAD"
#define PERCENT_CHANGE 99 // 99%

char connCtrl[CONNECTION_MAX];
connection_t conninfo[CONNECTION_MAX];
int numConnect = 0; 
int end_idx = 0;
sensor_t sensor;
sensor_t default_specs;
int send_fds[CONNECTION_MAX];
int listenfd2;
struct sockaddr_in server2;

void* handle_thread(void* connfd);
int handle_msg(char* msg, connection_t* connection);
void get_default_specs();
void gen_sensor_status();
void ctrl_sensor_status();
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
  memset(&sensor, 0, sizeof(sensor_t) );
  memset(&default_specs, 0, sizeof(sensor_t) );
  memset(&server, 0, sizeof(server) );
  memset(&client, 0, sizeof(client) );

  if (argc != 3) {
    fprintf(stderr, "Please write: ./sensor <ssid> <password>\n");
    exit(EXIT_FAILURE);
  }
  
  sensor.id = genID();
  strncpy(sensor.ssid, argv[1], SSID_LEN);
  strncpy(sensor.password, argv[2], PASSWORD_LEN);
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
  memset(&server2, 0, sizeof(server2) );
  server2.sin_family = AF_INET;
  server2.sin_port = htons(DEFAULT_PORT);
  server2.sin_addr.s_addr = htonl(INADDR_ANY);
  if ( (listenfd2 = socket(AF_INET, SOCK_STREAM, 0) ) == -1 ) {
      perror("\nsocket()");
      exit(EXIT_FAILURE);
  }
  if (bind(listenfd2, (struct sockaddr*) &server2, sizeof(server2) ) == -1) {
    perror("\nbind() 2");
    exit(EXIT_FAILURE);
  }
  if (listen(listenfd2, BACKLOG) == -1) {
    perror("\nlisten()");
    exit(EXIT_FAILURE);
  }
//-------------------------------------------------------------------------------  
  printf("Sensor started!\n");
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

  gen_sensor_status();
  // Create thread to control sensor status
  retval = pthread_create(&tid, NULL, &handle_thread, CONTROLLER_THREAD);
  if (retval!= 0) perror("\npthread_create()");
  // Create thread to send sensor's data to client
  retval = pthread_create(&tid, NULL, &handle_thread, DATA_SEND_THREAD);
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
  if (strcmp(arg,CONTROLLER_THREAD) == 0) {
    while (1) {
      ctrl_sensor_status();
      sleep(1);
    }
  } else if (strcmp(arg, DATA_SEND_THREAD) == 0) {
    while (1) {
      // int listenfd, connfd;
      // struct sockaddr_in server, client;
      // socklen_t sin_size = sizeof(struct sockaddr);
      // memset(&server, 0, sizeof(server) );
      
      // server.sin_family = AF_INET;
      // server.sin_port = htons(DEFAULT_PORT);
      // server.sin_addr.s_addr = htonl(INADDR_ANY);
      // if ( (listenfd = socket(AF_INET, SOCK_STREAM, 0) ) == -1 ) {
      //   perror("\nsocket()");
      //   exit(EXIT_FAILURE);
      // }
      // if (bind(listenfd, (struct sockaddr*) &server, sizeof(server) ) == -1) {
      //   perror("\nDATA_SEND_THREAD: bind()");
      //   exit(EXIT_FAILURE);
      // }
      // if (listen(listenfd, BACKLOG) == -1) {
      //   perror("\nlisten()");
      //   exit(EXIT_FAILURE);
      // }
      int listenfd = listenfd2, connfd;
      struct sockaddr_in client;
      socklen_t sin_size = sizeof(struct sockaddr);
      memset(&client, 0, sizeof(client) );
    //-------------------------------------------------------------------------------  
      char msg[MSG_SIZE];
      int ip = 0;
      int check = 0;
      struct pollfd listenfd_poll;
      listenfd_poll.fd = listenfd;
      listenfd_poll.events = POLLIN;
      int ready = 0;
      memset(msg, 0, MSG_SIZE);

      ready = poll(&listenfd_poll, 1, TIMEOUT);
      if (ready == -1) {
        perror("\npoll()");
      } else if (ready == 1) {
        connfd = accept(listenfd, (struct sockaddr*) &client, &sin_size);
        if (connfd == -1) {
          perror("\naccept()");
        }
        ip = client.sin_addr.s_addr;
        for (int i = 0; i <= end_idx; i++) {
          if (connCtrl[i] == 2 && ip == conninfo[i].ip) {
            send_fds[i] = connfd;
            connCtrl[i] = 3;
            check = 1;
            break;
          }
        }
        if (check == 0) close(connfd);
      }

      for (int i = 0; i <= end_idx; i++) {
        if(connCtrl[i] == 3) {
          snprintf(msg, sizeof(msg), "%d %d %d %d", sensor.humidity, sensor.nitrogen, sensor.phosphorus, sensor.potassium);
          send_msg(msg, send_fds[i]);
        }
      }
      sleep(sensor.response_time);
    }
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
    if (strcmp(password, sensor.password) == 0) {
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
// > 150 <status> <response_time>// GET_SUCCESS
  } else if (strcmp(command, "GET") == 0) {
    char sensor_data[81];
    snprintf(sensor_data, sizeof(sensor_data), "%s %d %d", GET_SUCCESS, sensor.status, sensor.response_time);
    send_msg(sensor_data, connfd);
    return 1;
// SET <response_time>
// > 171 // SET_SENSOR_SUCCESS
// > 203 // INVALID_ARGS
  } else if (strcmp(command, "SET") == 0) {
    int response_time;
    snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %%d%%c");
    retval = sscanf(msg, type_of_msg, &response_time, overcheck);
    if (retval!= 1) {
      send_msg(MSG_NOT_DETERMINED, connfd);
      return 1;
    }
    if (response_time < 0 || response_time > 100) {
      send_msg(INVALID_ARGS, connfd);
      return 1;
    } else {
      sensor.response_time = response_time;
      send_msg(SET_SENSOR_SUCCESS, connfd);
      return 0;
    }
// // simulation command
// UPDATE HUMID <HMAX>
// > 400 // UPDATE_SUCCESS

// UPDATE NPK+ <NMIN> <PMIN> <KMIN>
// > 400 // UPDATE_SUCCESS
// // humidity += (100-humidity)*0.5;
  } else if (strcmp(command, "UPDATE") == 0) {
    char cmd_arg[6];
    snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %%%ds%%c", 5);
    retval = sscanf(msg, type_of_msg, cmd_arg, overcheck);
    if (retval != 2 || overcheck != ' ') {
      send_msg(MSG_NOT_DETERMINED, connfd);
      return 1;
    }
    if (strcmp(cmd_arg, "HUMID") == 0) {
      int HMAX;
      snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %%*s %%d%%c");
      retval = sscanf(msg, type_of_msg, &HMAX, overcheck);
      if (retval != 1) {
        send_msg(MSG_NOT_DETERMINED, connfd);
        return 1;
      }
      for (int i = 0; i < CTRL_LOOP; i++) {
        sensor.humidity = HMAX;
      }
      send_msg(UPDATE_SUCCESS, connfd);
      return 0;
    } else if (strcmp(cmd_arg, "NPK+") == 0) {
      int NMIN, PMIN, KMIN;
      snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %%*s %%d %%d %%d%%c");
      retval = sscanf(msg, type_of_msg, &NMIN, &PMIN, &KMIN, overcheck);
      if (retval != 3) {
        send_msg(MSG_NOT_DETERMINED, connfd);
        return 1;
      }
      for (int i = 0; i < CTRL_LOOP; i++) {
        sensor.humidity += (100 - sensor.humidity)*0.5;
        sensor.nitrogen = NMIN;
        sensor.phosphorus = PMIN;
        sensor.potassium = KMIN;
      }
      send_msg(UPDATE_SUCCESS, connfd);
      return 0;
    } else {
      send_msg(UNKNOWN_COMMAND, connfd);
      return 1;
    }
// WRITE <humidity> <nitrogen> <phosphorus> <potassium>
// > 401 // WRITE_SUCCESS
  } else if (strcmp(command, "WRITE") == 0) {
    int humidity, nitrogen, phosphorus, potassium;
    snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %%d %%d %%d %%d%%c");
    retval = sscanf(msg, type_of_msg, &humidity, &nitrogen, &phosphorus, &potassium, overcheck);
    if (retval != 4) {
      send_msg(MSG_NOT_DETERMINED, connfd);
      return 1;
    }
    sensor.humidity = humidity;
    sensor.nitrogen = nitrogen;
    sensor.phosphorus = phosphorus;
    sensor.potassium = potassium;
    send_msg(WRITE_SUCCESS, connfd);
    return 0;
  } else {
    send_msg(UNKNOWN_COMMAND, connfd);
    return 1;
  }
}
void get_default_specs() {
//****************************************************************
// future feature: get default specs from file
  default_specs.response_time = 10;
}
void gen_sensor_status() {
  sensor.response_time = default_specs.response_time;
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
void reset_default_specs() {
  strcpy(sensor.ssid, default_specs.ssid);
  strcpy(sensor.password, default_specs.password);
  sensor.response_time = default_specs.response_time;
}
