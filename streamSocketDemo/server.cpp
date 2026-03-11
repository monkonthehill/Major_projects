#include <arpa/inet.h>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define MYPORT "3490"
#define BACKLOG 10

// void sigchld_handler(int s){
//     void(s);
//     // waitpid() might overwrite errno, so we save and restore it:
//     int saved_errno = errno;
//     while (waitpid(-1, NULL, WNOHANG)>0);
//     errno = saved_errno;
// }
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}
int main() {
  int socketfd, newfd;
  struct addrinfo hints, *res, *p;
  struct sockaddr_storage their_addr;
  socklen_t sin_size;
  char s[INET6_ADDRSTRLEN];
  int yes = 1;
  int rv;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (rv = getaddrinfo(NULL, MYPORT, &hints, &res) != 0) {
    std::cout << stderr << "getaddrinfo " << gai_strerror(rv);
    return 0;
  }
  for (p = res; p != 0; p->ai_next) {
    if (socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol) == -1) {
      perror("server:socket");
      continue;
    }

    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) ==
        -1) {
      perror("setsockopt");
      exit(1);
    }
    if (bind(socketfd, p->ai_addr, p->ai_addrlen) == -1) {
      perror("server: bind");
      continue;
    }
    break;
  }
  freeaddrinfo(res);
  if (p == NULL) {
    std::cout << stderr << "server failed fo bind";
  }
  if (listen(socketfd, BACKLOG)) {
    perror("listen");
  }
  std::cout << "server waiting for a connection" << std::endl;
  while (true) {
    sin_size = sizeof their_addr;
    newfd = accept(socketfd, (struct sockaddr *)&their_addr, &sin_size);
    if (newfd == -1) {
      perror("accept");
      continue;
    }
  }
}
