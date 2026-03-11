#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
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
    std::cerr << "usage: client hostname\n";
    exit(1);
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(argv[1], PORT, &hints, &res)) != 0) {
    std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
    return 1;
  }

  for (p = res; p != NULL; p = p->ai_next) {

    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s,
              sizeof s);

    std::cout << "client: trying to connect to " << s << std::endl;

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("client: connect");
      continue;
    }

    break;
  }

  if (p == NULL) {
    std::cerr << "client: failed to connect\n";
    return 2;
  }

  freeaddrinfo(res);

  if ((numberbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
    perror("recv");
    exit(1);
  }

  buf[numberbytes] = '\0';

  std::cout << "client: received '" << buf << "'" << std::endl;

  close(sockfd);

  return 0;
}
