#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

bool isIpv4(char*);

int main(int argc, char **argv){
  char *input = argv[1];
  struct addrinfo hints, *result = NULL, *rs = NULL;
  struct sockaddr_in *ptrAddr = NULL;
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  int ret;

  if(argc != 2){
    fprintf(stderr, "Please write: %s <domain/IP_address>\n", argv[0]);
    return -1;
  }
  if(isIpv4(input)){
  // get domain info by ipv4 address
    //convert IP address from dots-and-number string to a struct in_addr
    if(inet_pton(AF_INET, input, &address.sin_addr) == 0){
      perror("inet_pton() failed");
      return -1;
    }
    // search IP information
    char hostname[NI_MAXHOST];
    char servInfo[NI_MAXSERV];
    ret = getnameinfo((struct sockaddr *) &address, sizeof(struct sockaddr), hostname, NI_MAXHOST, servInfo, NI_MAXSERV, 0);
    if(ret != 0){
      fprintf(stderr, "getnameinfo() > Failed: %s\n", gai_strerror(ret));
      return -1;
    }
    if(strlen(hostname) == 0 || strcmp(hostname,input)==0)
      printf("Not found information\n");
    else
      printf("Result domain:\n%s\n", hostname);
  }

  // get ipv4 address info by name
  else{
    // because of getaddrinfo() return Internet service provider's ip address when input is invalid domain
    // I create random invalid domain to get ISP's ip address to check the invalid domain name
    const char invalidDomain[] = "www.1qaz4esz2wsx5rdx3edc6tfc5tgb8uhb8ik0ok.com";
    ret = getaddrinfo(invalidDomain, NULL, &hints, &result);
    ptrAddr = (struct sockaddr_in*) result->ai_addr;
    struct in_addr ispAddr = ptrAddr->sin_addr;
    // get ipv4 address by input domain name
    ret = getaddrinfo(input, NULL, &hints, &result);
    // if ret = -2 print "Not found information" instead of "Name or service not known" (=get_strerror(8))
    if(ret == -2){
      printf("Not found information\n");
      return -1;
    }
    if(ret != 0){
      fprintf(stderr, "getaddrinfo() > Failed: %s\n", gai_strerror(ret));
      return -1;
    }
    // check whether result ip address is invalid domain name or not
    ptrAddr = (struct sockaddr_in*) result->ai_addr;
    if(ptrAddr->sin_addr.s_addr == ispAddr.s_addr){
      printf("Not found information\n");
      return -1;
    }

    printf("Result IP:\n");
    for(rs = result; rs != NULL; rs = rs->ai_next){
      char strIP[INET_ADDRSTRLEN];
      ptrAddr = (struct sockaddr_in*) rs->ai_addr;
      inet_ntop(AF_INET, &ptrAddr->sin_addr, strIP, sizeof(strIP));
      printf("%s\n", strIP);
    }
    freeaddrinfo(result);
  }
  return 0;
}
/*
@function isIpv4: check the string is an ipv4 address or not
@param source: A pointer to a input string.
@return: FALSE if input string is not ipv4 address
        TRUE if it is an ipv4 address
*/
bool isIpv4(char *str){
  if (str == NULL) {
      return false;
  }
  // Create a copy of the input string to avoid modifying the original
  char *copy = strdup(str);
  if (copy == NULL) {
      perror("Memory allocation error");
      exit(EXIT_FAILURE);
  }
  char *token = strtok(copy, ".");
  int segs = 0; // Count of segments

  while(token != NULL) {
      int intV = atoi(token);
      // atoi(token) return 0 if token is not a integer
      // when token = "0": strlen(token) = 1 and token[0] == '0')
      // return faile if token is not a integer ignore case token = "0"
      if(intV == 0 && (strlen(token) > 1 || token[0] != '0')){
        free(copy);
        return false;
      }
      if(intV < 0 || intV > 255) {
          free(copy);
          return false;
      }
      segs++;
      token = strtok(NULL, ".");
      // IPv4 must have 4 segments
      if(segs > 4 && token != NULL) {
          free(copy);
          return false;
      }
  }
  free(copy);
  return (segs == 4);
}

