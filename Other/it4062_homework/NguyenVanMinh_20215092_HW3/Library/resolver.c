#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <arpa/inet.h>

/* ------------------------------
@function getDomainName: this function is used to request domain name using getnameinfo()
@param input: IPv4 address
       output: Pointer to Domain name if success
@return: 0 if success
        -1 if an error occur or can not get any infomation
        -2 if not found infomation
        -3 if input is invalid IPv4 address
*/
int getDomainName(const char* input, char** output) {
  //convert IP address from dots-and-number string to a struct in_addr
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  int ret;
  int intV = inet_pton(AF_INET, input, &address.sin_addr);
  if (intV == 0) return -3; // Warning: Invalid IPv4 address
  else if (intV < 0) return -1; // Error: inet_pton() has failed

  // search IP information
  // NI_MAXHOST value is 1025
  char *hostname = (char*) malloc(NI_MAXHOST);
  char servInfo[NI_MAXSERV];
  ret = getnameinfo( (struct sockaddr *) &address, sizeof(struct sockaddr), hostname, NI_MAXHOST, servInfo, NI_MAXSERV, 0);
  if (ret != 0) {
    free(hostname);
    return -1; // getnameinfo() has failed
  }
  if (strlen(hostname) == 0 || strcmp(hostname,input) == 0) {
    free(hostname);
    return -2; // Not found information
  }
  *output = hostname;
  return 0;
}
/* ------------------------------
@function getIPAdress: this function is used to request IPv4 address using getaddrinfo()
@param input: Domain name
       output: Pointer to linked-list containing host's IP information
@return: 0 if success
        -1 if an error occur or can not get any infomation
        -2 if not found infomation
*/
int getIPAddress(const char* input, struct addrinfo** output) {
  struct addrinfo hints;
  struct addrinfo *result = NULL;
  struct sockaddr_in *ptrAddr = NULL;
  int ret;
  memset(&hints, 0, sizeof(struct addrinfo) );
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  // get IPv4 address by input domain name
  ret = getaddrinfo(input, NULL, &hints, &result);
  // if ret = -2 print "Not found information" instead of "Name or service not known" (=get_strerror(-2) )
  if (ret == -2) return -2; // Not found information
  if (ret != 0) return -1; // getaddrinfo() has failed

  char strIP[INET_ADDRSTRLEN];
  ptrAddr = (struct sockaddr_in*) result->ai_addr;
  inet_ntop(AF_INET, &ptrAddr->sin_addr, strIP, sizeof(strIP) );
  if (strcmp(strIP, input) == 0) return -2; // Not found information
  *output = result;
  return 0;
}

/* ------------------------------
@function resolver: translate request for Domain name into IPv4 address or IPv4 address to Domain name
@param input: Domain name or IPv4 address
@return: 0 if success to get IPv4 address from Domain name
         1 if success to get Domain name from IPv4 address
        -1 if an error occur or can not get any infomation
        -2 if not found infomation
        -3 if input is invalid IPv4 address
*/
int resolver(const char* input, void** info) {
  int hasAlpha = 0;
  int retval = -1;
  for (int i = 0; input[i] != '\0'; i++)
    if (isalpha(input[i]) ) {
      hasAlpha = 1;
      break;
    }

  if (hasAlpha == 0) {
    char *domain = NULL;
    retval = getDomainName(input, &domain);
    if (retval == 0) {
      *info = domain;
      return 1;
    } return retval;
  } else {
    struct addrinfo *ipaddr = NULL;
    retval = getIPAddress(input, &ipaddr);
    if (retval == 0) {
      *info = ipaddr;
      return 0;
    } else return retval;
  }

  return -1;
}
