#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MYPORT "3490"
#define BACKLOG 10

void sigchld_handler(int s) {
  (void)s;
  int saved_errno = errno;
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
  errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET)
    return &(((struct sockaddr_in *)sa)->sin_addr);

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main() {
  int sockfd, newfd;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr;
  socklen_t sin_size;
  struct sigaction sa;
  int yes = 1;
  char s[INET6_ADDRSTRLEN];
  int rv;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;     // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP
  hints.ai_flags = AI_PASSIVE;     // use my IP

  if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
    std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
    return 1;
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {

    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo);

  if (p == NULL) {
    std::cerr << "server: failed to bind\n";
    return 2;
  }

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  // handle zombie processes
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  std::cout << "server: waiting for connections...\n";

  while (true) {
    sin_size = sizeof their_addr;

    newfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (newfd == -1) {
      perror("accept");
      continue;
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),
              s, sizeof s);

    std::cout << "server: got connection from " << s << std::endl;

    if (!fork()) {
      close(sockfd);

      if (send(newfd, "Hello, world!", 13, 0) == -1)
        perror("send");

      close(newfd);
      exit(0);
    }

    close(newfd);
  }

  return 0;
}
