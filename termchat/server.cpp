#include <arpa/inet.h>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#define BACKLOGS 10

int serv_disc() {
  int serv_fd;

  if ((serv_fd = socket(AF_INET6, SOCK_STREAM, 0)) == -1) {
    perror("socket initialization failed");
    exit(1);
  }

  return serv_fd;
}

int client_disc(int sock_fd, sockaddr_in6 *address) {
  int sock;
  socklen_t addrlen = sizeof(*address);

  sock = accept(sock_fd, (sockaddr *)address, &addrlen);

  if (sock == -1) {
    perror("accept request failed");
  }

  return sock;
}

int main() {
  std::cout << "started sucessfully !!!" << std::endl;
  int sock_fd, sock;
  sock_fd = serv_disc();
  char buffer[1024] = {0};
  sockaddr_in6 address;
  std::vector<pollfd> fds;

  address.sin6_family = AF_INET6;
  address.sin6_port = htons(8080);
  address.sin6_addr = in6addr_any;

  pollfd server_poll;
  server_poll.fd = sock_fd;
  server_poll.events = POLLIN;
  server_poll.revents = 0;

  int opt = 1;
  setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  if (bind(sock_fd, (sockaddr *)&address, sizeof(address)) == -1) {
    perror("binding failed");
    exit(1);
  }

  if (listen(sock_fd, BACKLOGS) == -1) {
    perror("listening failed");
    exit(1);
  }

  fds.push_back(server_poll);
  while (true) {
    int ret = poll(fds.data(), fds.size(), -1);
    if (ret < 0) {
      perror("poll failed");
      break;
    }

    for (int i = 0; i < fds.size(); i++) {

      if (fds[i].revents & (POLLIN | POLLHUP | POLLERR)) {

        if (fds[i].fd == sock_fd) {
          sockaddr_in6 client_addr;
          socklen_t addrlen = sizeof(client_addr);
          int new_client = accept(sock_fd, (sockaddr *)&client_addr, &addrlen);

          if (new_client == -1) {
            perror("accept failed");
            continue;
          }

          pollfd new_fd = {new_client, POLLIN, 0};
          fds.push_back(new_fd);
          std::cout << "Someone joined !!" << std::endl;
        }

        else {
          int bytes = read(fds[i].fd, buffer, sizeof(buffer));

          if (bytes <= 0) {
            std::cout << "Client disconnected\n";
            close(fds[i].fd);
            fds.erase(fds.begin() + i);
            i--;
            continue;
          }

          std::cout << "Client: " << buffer << std::endl;

          for (int j = 0; j < fds.size(); j++) {
            if (fds[j].fd != sock_fd && fds[j].fd != fds[i].fd) {
              send(fds[j].fd, buffer, strlen(buffer), 0);
            }
          }
        }
      }
    }
  }
  close(sock);
  close(sock_fd);
}
