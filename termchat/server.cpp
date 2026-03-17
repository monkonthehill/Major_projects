#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#define BACKLOGS 10

struct Client {
  int fd;
  std::string room;
};

Client *get_client(int fd, std::vector<Client> &clients) {
  for (auto &c : clients) {
    if (c.fd == fd)
      return &c;
  }
  return nullptr;
}

int main() {
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1) {
    perror("socket");
    return 1;
  }

  int opt = 1;
  setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_port = htons(8080);
  address.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock_fd, (sockaddr *)&address, sizeof(address)) == -1) {
    perror("bind");
    return 1;
  }

  if (listen(sock_fd, BACKLOGS) == -1) {
    perror("listen");
    return 1;
  }

  std::vector<pollfd> fds;
  std::vector<Client> clients;

  fds.push_back({sock_fd, POLLIN, 0});

  char buffer[1024];

  while (true) {
    if (poll(fds.data(), fds.size(), -1) < 0) {
      perror("poll");
      break;
    }

    for (int i = 0; i < fds.size(); i++) {
      if (fds[i].revents & (POLLIN | POLLHUP | POLLERR)) {

        // NEW CONNECTION
        if (fds[i].fd == sock_fd) {
          sockaddr_in client_addr{};
          socklen_t len = sizeof(client_addr);

          int new_fd = accept(sock_fd, (sockaddr *)&client_addr, &len);
          if (new_fd == -1) {
            perror("accept");
            continue;
          }

          fds.push_back({new_fd, POLLIN, 0});

          Client c{new_fd, "lobby"};
          clients.push_back(c);

          std::cout << "Client connected\n";
        }

        // CLIENT MESSAGE
        else {
          memset(buffer, 0, sizeof(buffer));
          int bytes = read(fds[i].fd, buffer, sizeof(buffer));

          if (bytes <= 0) {
            std::cout << "Client disconnected\n";
            close(fds[i].fd);

            // remove from fds
            fds.erase(fds.begin() + i);
            i--;

            // remove from clients
            for (int k = 0; k < clients.size(); k++) {
              if (clients[k].fd == fds[i].fd) {
                clients.erase(clients.begin() + k);
                break;
              }
            }

            continue;
          }

          std::string msg(buffer);
          Client *sender = get_client(fds[i].fd, clients);

          // JOIN COMMAND
          if (msg.rfind("/join ", 0) == 0) {
            std::string room = msg.substr(6);
            if (sender)
              sender->room = room;
            continue;
          }

          // BROADCAST IN ROOM
          for (auto &c : clients) {
            if (c.fd != fds[i].fd && c.room == sender->room) {
              send(c.fd, buffer, strlen(buffer), 0);
            }
          }
        }
      }
    }
  }

  close(sock_fd);
}
