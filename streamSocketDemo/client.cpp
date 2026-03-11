#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define PORT "3490"
#define MAXDATASIZE 100

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
  int sockfd, numberbytes;
  char buf[MAXDATASIZE];
  struct addrinfo hints, *res, *p;
  int rv;
  char s[INET6_ADDRSTRLEN];

  if (argc != 2) {
    std::cout << stderr << "usage : client hostname" << std::endl;
    exit(0);
  }
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if ((rv = getaddrinfo(argv[1], PORT, &hints, &res)) != 0) {
    std::cout << stderr, "getaddrinfo", gai_strerror(rv);
    return 1;
  }
  for (p = res; p != NULL; p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("client : socketfd");
      continue;
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s,
              sizeof s);
    std::cout << "client : trying connect to " << s << std::endl;
    freeaddrinfo(res);
    if ((numberbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
      perror("recv");
      exit(1);
    }
    buf[numberbytes] = '\0';
    std::cout << "client : received " << buf;
    close(sockfd);
    return 0;
  }
}
