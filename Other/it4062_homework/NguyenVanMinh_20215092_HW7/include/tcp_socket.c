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
    fprintf(stderr, "send_msg(): message lenght must be less than %d character\n", MSG_SIZE - DELIMITER_SIZE);
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
@function: receive 1 message from connfdet
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
    case 301: return M301; break;
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
