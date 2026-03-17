
#include <arpa/inet.h> // inet_ntop(), inet_pton()
#include <cstddef>
#include <cstring> // memset()
#include <iostream>
#include <netdb.h>      // addrinfo, getaddrinfo()
#include <netinet/in.h> // sockaddr_in, sockaddr_in6
#include <sys/socket.h> // socket structures
#include <sys/types.h>
#define MYPORT "3490"
#define BACKLOG 10

int main() {
  struct addrinfo hints, *res;
  struct sockaddr_storage their_addr;
  int sockfd, new_fd;
  char *msg = "Hii there";
  int len = strlen(msg);
  socklen_t addrsize;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_socktype = SOCK_STREAM;
  getaddrinfo(NULL, MYPORT, &hints, &res);
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  bind(sockfd, res->ai_addr, res->ai_addrlen);
  listen(sockfd, BACKLOG);

  addrsize = sizeof their_addr;
  new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addrsize);
  int byte_sent = send(new_fd, msg, len, 0);
}
