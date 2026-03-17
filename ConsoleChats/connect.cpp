#include <arpa/inet.h> // inet_ntop(), inet_pton()
#include <cstring>     // memset()
#include <iostream>
#include <netdb.h>      // addrinfo, getaddrinfo()
#include <netinet/in.h> // sockaddr_in, sockaddr_in6
#include <sys/socket.h> // socket structures
#include <sys/types.h>

int main() {
  int dep = 0;
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  getaddrinfo("www.google.com", "3490", &hints, &res);

  dep = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  connect(dep, res->ai_addr, res->ai_addrlen);
  struct sockaddr *generic_addr;
}
