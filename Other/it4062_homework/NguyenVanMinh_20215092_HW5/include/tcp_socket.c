#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include "tcp_socket.h"

/*
@function: send message to client
@parameter [IN] msg: message to send
           [IN] conn_sock: socket connects to client
@return: true if success
         false if fail
*/
bool send_msg(char* message, int conn_sock) {
  char send_data[MSG_SIZE];
  int sent_bytes;
  sprintf(send_data, "%s%s", message, DELIMITER);
  sent_bytes = send(conn_sock, send_data, sizeof(send_data), 0);
  if(sent_bytes < 0) {
    perror("\nsend()");
    close(conn_sock);
    return false;
  }
  return true;
}
/*
@function: receive 1 message from conn_socket
@parameter [IN] conn_sock: socket connects to server
           [IN] recv_buffer: save remaining data after receiving message
           [OUT] message: save message have been received
@return: 0 if success
         1 if success and unprocessed information will be put into recv_buffer
        -1 if fail or connection closed
*/
int get_msg(int conn_sock, char* message, char* recv_buffer) {
	char recv_data[MSG_SIZE], recv_msg[MSG_SIZE];
  int received_bytes;
  memset(recv_msg, 0, MSG_SIZE);

  do {
    if (recv_buffer[0] != '\0') {
      memmove(recv_data, recv_buffer, strlen(recv_buffer) );
      received_bytes = strlen(recv_buffer);
      memset(recv_buffer, 0, MSG_SIZE);
    } else {
      received_bytes = recv(conn_sock, recv_data, MSG_SIZE -1, 0);
      if (received_bytes < 0) {
        if (errno == ECONNRESET) return -1;
        perror("\nrecv()");
        return -1;
      }
      else if (received_bytes == 0) {
        return -1;
      }
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
        memmove(message, recv_msg, strlen(recv_msg) + 1 );
        optr = ptr + strlen(DELIMITER);
        if (*optr != '\0') {
          memmove(recv_buffer+strlen(recv_buffer), optr, strlen(optr) );
          return 1;
        } else return 0;
      }
    }
  } while (received_bytes > 0);
  return -1;
}
/* Return a string describing the meaning of the server's message code */
char* strmsg(char* msg_code) {
  char* ptr;
  int code = strtol(msg_code, &ptr, 10);
  if (ptr != msg_code + 3) return NULL;
  switch (code) {
    case 100: return M100; break;
    case 110: return M110; break;
    case 120: return M120; break;
    case 130: return M130; break;
    case 211: return M211; break;
    case 212: return M212; break;
    case 213: return M213; break;
    case 221: return M221; break;
    case 300: return M300; break;
    default: return NULL;
  }
}
/* Print message by message code */
bool print_msg(char* msg_code) {
  if (msg_code[0] != '1')
    printf("!!%s!!\n", strmsg(msg_code) );
  else printf("%s\n", strmsg(msg_code) );
  return true;
}
