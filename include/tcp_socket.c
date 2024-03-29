#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include "tcp_socket.h"

/*
@function: send message to client
@parameter [IN] msg: message to send
           [IN] connfd: socket connects to client
@return: true if success
         false if fail
*/
bool send_msg(char* message, int connfd) {
  char send_data[MSG_SIZE];
  int sent_bytes;
  unsigned int msglen = (unsigned int) strlen(message);
  if (msglen > MSG_SIZE - DELIMITER_SIZE) {
    fprintf(stderr, "send_msg(): message length must be less than %d character\n", MSG_SIZE - DELIMITER_SIZE);
    return false;
  }
  memmove(send_data, message, msglen);
  memmove(send_data + msglen, DELIMITER, DELIMITER_SIZE);
  sent_bytes = send(connfd, send_data, msglen + DELIMITER_LEN, 0);
  if(sent_bytes < 0) {
    perror("\nsend()");
    close(connfd);
    return false;
  }
  return true;
}
/*
@function: receive 1 message from connfd
@parameter [IN] connfd: socket connects to server
           [IN] recv_buffer: save remaining data after receiving message
           [OUT] message: save message have been received
@return: 0 if success
         1 if success and unprocessed information will be put into recv_buffer
        -1 have error
        -2 Connection closed
        -3 Message overlength
*/

int get_msg(int connfd, char* message, char* recv_buffer) {
	char recv_data[MSG_SIZE], recv_msg[MSG_SIZE];
  int received_bytes;
  recv_data[0] = '\0';
  memset(recv_msg, 0, MSG_SIZE);

  do {
    if (recv_buffer[0] != '\0') { // get residual messages from recv_buffer
      memmove(recv_data, recv_buffer, strlen(recv_buffer) + 1);
      received_bytes = strlen(recv_buffer);
      recv_buffer[0] = '\0';
    } else {
      received_bytes = recv(connfd, recv_data, MSG_SIZE - 1, 0);
      if (received_bytes <= 0) {
        if (received_bytes == 0 || errno == ECONNRESET) return -2; // connection closed
        else {
          perror("\nrecv()");
          return -1;
        }
      }
    }
    recv_data[received_bytes] = '\0';
    char* ptr = strstr(recv_data, DELIMITER);
    char* optr = recv_data;
    while (1) {
      if (ptr == NULL) {
        if ( strlen(recv_msg) + strlen(optr) > MSG_SIZE - 1) return -3; // message overlength
        memmove(recv_msg + strlen(recv_msg), optr, strlen(optr) );
        break;
      } else {
        if ( strlen(recv_msg) + ptr - optr > MSG_SIZE - 1) return -3;
        memmove(recv_msg + strlen(recv_msg), optr, ptr - optr);
        memmove(message, recv_msg, strlen(recv_msg) + 1 );
        optr = ptr + DELIMITER_LEN;
        if (*optr != '\0') {
          memmove(recv_buffer, optr, strlen(optr) + 1);
          return 1;
        } else return 0;
      }
    }
  } while (received_bytes > 0);
  return -1;
}
//****************************************************************
int get_msg_t(int connfd, char* message, char* recv_buffer, int timeout) {
	char recv_data[MSG_SIZE], recv_msg[MSG_SIZE];
  int received_bytes;
  recv_data[0] = '\0';
  memset(recv_msg, 0, MSG_SIZE);

  do {
    if (recv_buffer[0] != '\0') { // get residual messages from recv_buffer
      memmove(recv_data, recv_buffer, strlen(recv_buffer) + 1);
      received_bytes = strlen(recv_buffer);
      recv_buffer[0] = '\0';
    } else {
      received_bytes = recv(connfd, recv_data, MSG_SIZE - 1, 0);
      if (received_bytes <= 0) {
        if (received_bytes == 0 || errno == ECONNRESET) return -2; // connection closed
        else {
          perror("\nrecv()");
          return -1;
        }
      }
    }
    recv_data[received_bytes] = '\0';
    char* ptr = strstr(recv_data, DELIMITER);
    char* optr = recv_data;
    while (1) {
      if (ptr == NULL) {
        if ( strlen(recv_msg) + strlen(optr) > MSG_SIZE - 1) return -3; // message overlength
        memmove(recv_msg + strlen(recv_msg), optr, strlen(optr) );
        break;
      } else {
        if ( strlen(recv_msg) + ptr - optr > MSG_SIZE - 1) return -3;
        memmove(recv_msg + strlen(recv_msg), optr, ptr - optr);
        memmove(message, recv_msg, strlen(recv_msg) + 1 );
        optr = ptr + DELIMITER_LEN;
        if (*optr != '\0') {
          memmove(recv_buffer, optr, strlen(optr) + 1);
          return 1;
        } else return 0;
      }
    }
  } while (received_bytes > 0);
  return -1;
}
char* str_to_msg(char* msg_code) {
  char* ptr;
  int code = strtol(msg_code, &ptr, 10);
  if (ptr != msg_code + 3) return NULL;
  switch (code) {
    case 100: return M100;
    case 101: return M101;
    case 102: return M102;
    case 103: return M103;
    case 104: return M104;
    case 105: return M105;
    case 110: return M110;
    case 120: return M120;
    case 121: return M121;
    case 130: return M130;
    case 140: return M140;
    case 141: return M141;
    case 150: return M150;
    case 160: return M160;
    case 161: return M161;
    case 162: return M162;
    case 171: return M171;
    case 172: return M172;
    case 173: return M173;
    case 174: return M174;
    case 180: return M180;
    case 181: return M181;
    case 190: return M190;
    case 200: return M200;
    case 201: return M201;
    case 202: return M202;
    case 203: return M203;
    case 220: return M220;
    case 221: return M221;
    case 230: return M230;
    case 260: return M260;
    case 261: return M261;
    case 262: return M262;
    case 300: return M300;
    case 301: return M301;
    case 302: return M302;
    case 400: return M400;
    case 401: return M401;
    default: return NULL;
  }
}
/* Print message by message code */
bool print_msg(char* msg_code) {
  if (msg_code[0] != '1')
    printf("!!%s!!\n", str_to_msg(msg_code) );
  else printf("%s\n", str_to_msg(msg_code) );
  return true;
}
//********************************
//********************************
/*
@function: Echo and receive message from server
@parameter [IN] connfd: socket descriptor that connect to server
*/
int echo_server(int connfd) {
	char send_data[MSG_SIZE], recv_data[MSG_SIZE];
	char recv_msg[MSG_SIZE];
  int sent_bytes, received_bytes;
  memset(send_data, 0, MSG_SIZE);
  memset(recv_msg, 0, MSG_SIZE);

  while (1) {
    printf("Enter message: ");
    fgets(send_data, MSG_SIZE - DELIMITER_SIZE, stdin);
    if (send_data[0] == '\n') break; // cancel send
    memmove(send_data+strlen(send_data)-1, DELIMITER, DELIMITER_SIZE);
    sent_bytes = send(connfd, send_data, strlen(send_data), 0);
    if(sent_bytes < 0) {
      perror("\nsend()");
      close(connfd);
      return -1;
    }
  }

  do {
    received_bytes = recv(connfd, recv_data, MSG_SIZE -1, 0);
    if (received_bytes < 0) {
      perror("\nrecv()");
      close(connfd);
      return -1;
    } else if (received_bytes == 0) {
      printf("Connection closed.\n");
      return -1;
    }
    recv_data[received_bytes] = '\0';
    char *ptr = strstr(recv_data, DELIMITER);
    char *optr = recv_data;
    while (1) {
      if (ptr == NULL) {
        memmove(recv_msg+strlen(recv_msg), optr, strlen(optr) );
        break;
      } else {
        memmove(recv_msg+strlen(recv_msg), optr, ptr - optr);
        printf("Received message: %s#\n", recv_msg);
        memset(recv_msg, 0, MSG_SIZE);
        optr = ptr + DELIMITER_LEN;
        ptr = strstr(ptr + 1, DELIMITER);
      }
    }
  } while (received_bytes > 0);

	close(connfd);
	return 0;
}
/*
@function: Receive and echo message to client
@parameter [IN] connfd: socket descriptor that connects to client
*/
int echo_client(int connfd) {
	char recv_data[MSG_SIZE];
	char recv_msg[MSG_SIZE];
  int sent_bytes, received_bytes;

  do {
    received_bytes = recv(connfd, recv_data, MSG_SIZE -1, 0);
    if (received_bytes < 0) {
      perror("\nrecv()");
      close(connfd);
      return -1;
    } else if (received_bytes == 0) {
      printf("Connection closed.\n");
      return -1;
    }
    recv_data[received_bytes] = '\0';
    char *ptr = strstr(recv_data, DELIMITER);
    char *optr = recv_data;
    while (1) {
      if (ptr == NULL) {
        memmove(recv_msg+strlen(recv_msg), optr, strlen(optr) );
        break;
      } else {
        memmove(recv_msg+strlen(recv_msg), optr, ptr - optr);
        printf("Received message: %s#\n", recv_msg);
        memset(recv_msg, 0, MSG_SIZE);
        optr = ptr + DELIMITER_LEN;
        ptr = strstr(ptr + 1, DELIMITER);
      }
    }
    // Echo to client
    sent_bytes = send(connfd, recv_data, strlen(recv_data), 0);
    if(sent_bytes < 0) {
      perror("\nsend()");
      close(connfd);
      exit(1);
    }
  } while (received_bytes > 0);

	close(connfd);
	return 0;
}
/*
@function: Receive and echo message to client use thread
@parameter [IN] connfd: socket descriptor that connects to client
*/
void* echo_thread(void* inp_connfd) {
  pthread_detach( pthread_self() );
  int connfd = *( (int*) inp_connfd);
	char recv_data[MSG_SIZE];
	char recv_msg[MSG_SIZE];
  int sent_bytes, received_bytes;

  do {
    received_bytes = recv(connfd, recv_data, MSG_SIZE -1, 0);
    if (received_bytes < 0) {
      perror("\nrecv()");
      close(connfd);
    } else if (received_bytes == 0) {
      printf("Connection closed.\n");
    }
    recv_data[received_bytes] = '\0';
    char *ptr = strstr(recv_data, DELIMITER);
    char *optr = recv_data;
    while (1) {
      if (ptr == NULL) {
        memmove(recv_msg+strlen(recv_msg), optr, strlen(optr) );
        break;
      } else {
        memmove(recv_msg+strlen(recv_msg), optr, ptr - optr);
        printf("Received message: %s#\n", recv_msg);
        memset(recv_msg, 0, MSG_SIZE);
        optr = ptr + DELIMITER_LEN;
        ptr = strstr(ptr + 1, DELIMITER);
      }
    }
    // Echo to client
    sent_bytes = send(connfd, recv_data, strlen(recv_data), 0);
    if(sent_bytes < 0) {
      perror("\nsend()");
      close(connfd);
    }
  } while (received_bytes > 0);

	close(connfd);
  return NULL;
}

