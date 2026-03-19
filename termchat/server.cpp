#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#define BACKLOGS 10

struct Client {
  int fd;
  std::string username;
  std::string room; // "GLOBAL" or room name
};

Client *get_client(int fd, std::vector<Client> &clients) {
  for (auto &c : clients) {
    if (c.fd == fd)
      return &c;
  }
  return nullptr;
}

int main() {
  std::cout << "Server started...\n";

  int sock_fd = socket(AF_INET6, SOCK_STREAM, 0);
  if (sock_fd == -1) {
    perror("socket failed");
    return 1;
  }

  int opt = 1;
  setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  sockaddr_in6 address{};
  address.sin6_family = AF_INET6;
  address.sin6_port = htons(8080);
  address.sin6_addr = in6addr_any;

  if (bind(sock_fd, (sockaddr *)&address, sizeof(address)) == -1) {
    perror("bind failed");
    return 1;
  }

  if (listen(sock_fd, BACKLOGS) == -1) {
    perror("listen failed");
    return 1;
  }

  std::vector<pollfd> fds;
  std::vector<Client> clients;

  fds.push_back({sock_fd, POLLIN, 0});

  char buffer[1024];

  while (true) {
    int ret = poll(fds.data(), fds.size(), -1);
    if (ret < 0) {
      perror("poll failed");
      break;
    }

    for (int i = 0; i < fds.size(); i++) {

      if (!(fds[i].revents & (POLLIN | POLLHUP | POLLERR)))
        continue;

      // ================= NEW CONNECTION =================
      if (fds[i].fd == sock_fd) {
        sockaddr_in6 client_addr;
        socklen_t addrlen = sizeof(client_addr);

        int new_client = accept(sock_fd, (sockaddr *)&client_addr, &addrlen);
        if (new_client == -1) {
          perror("accept failed");
          continue;
        }

        // Prompt username
        std::string prompt = "Enter username:\n";
        send(new_client, prompt.c_str(), prompt.size(), 0);

        fds.push_back({new_client, POLLIN, 0});
        std::cout << "New client connected\n";
      }

      // ================= CLIENT MESSAGE =================
      else {
        int bytes = read(fds[i].fd, buffer, sizeof(buffer) - 1);

        if (bytes <= 0) {
          Client *client = get_client(fds[i].fd, clients);

          if (client) {
            std::cout << client->username << " disconnected\n";

            // Notify others in the same room
            std::string leave_msg = client->username + " left the room\n";
            for (int j = 0; j < fds.size(); j++) {
              if (fds[j].fd == sock_fd || fds[j].fd == fds[i].fd)
                continue;

              Client *other = get_client(fds[j].fd, clients);
              if (other && other->room == client->room) {
                send(fds[j].fd, leave_msg.c_str(), leave_msg.size(), 0);
              }
            }
          }

          close(fds[i].fd);

          clients.erase(
              std::remove_if(clients.begin(), clients.end(),
                             [&](Client &c) { return c.fd == fds[i].fd; }),
              clients.end());

          fds.erase(fds.begin() + i);
          i--;
          continue;
        }

        buffer[bytes] = '\0';
        std::string msg(buffer);

        // Remove newline if present
        if (!msg.empty() && msg.back() == '\n') {
          msg.pop_back();
        }

        Client *client = get_client(fds[i].fd, clients);

        // ================= REGISTRATION =================
        if (client == nullptr) {
          std::string username = msg;

          // Trim whitespace
          username.erase(0, username.find_first_not_of(" \n\r\t"));
          username.erase(username.find_last_not_of(" \n\r\t") + 1);

          if (username.empty())
            continue;

          clients.push_back({fds[i].fd, username, "GLOBAL"});
          std::cout << username << " joined GLOBAL room\n";

          // Welcome message
          std::string welcome = "System: Welcome " + username + "!\n";
          send(fds[i].fd, welcome.c_str(), welcome.size(), 0);

          // Notify others
          std::string join_msg = "System: " + username + " joined GLOBAL\n";
          for (int j = 0; j < fds.size(); j++) {
            if (fds[j].fd == sock_fd || fds[j].fd == fds[i].fd)
              continue;

            Client *other = get_client(fds[j].fd, clients);
            if (other && other->room == "GLOBAL") {
              send(fds[j].fd, join_msg.c_str(), join_msg.size(), 0);
            }
          }
          continue;
        }

        // ================= COMMANDS =================

        // /join roomname
        if (msg.rfind("/join ", 0) == 0) {
          std::string new_room = msg.substr(6);

          // Trim whitespace
          new_room.erase(0, new_room.find_first_not_of(" \n\r\t"));
          new_room.erase(new_room.find_last_not_of(" \n\r\t") + 1);

          if (new_room.empty()) {
            std::string err = "Error: Room name cannot be empty\n";
            send(client->fd, err.c_str(), err.size(), 0);
            continue;
          }

          std::cout << client->username << " moved from " << client->room
                    << " to " << new_room << "\n";

          // Notify old room
          std::string leave_msg =
              "System: " + client->username + " left the room\n";
          for (int j = 0; j < fds.size(); j++) {
            if (fds[j].fd == sock_fd || fds[j].fd == fds[i].fd)
              continue;

            Client *other = get_client(fds[j].fd, clients);
            if (other && other->room == client->room) {
              send(fds[j].fd, leave_msg.c_str(), leave_msg.size(), 0);
            }
          }

          // Update room
          client->room = new_room;

          // Notify new room
          std::string join_msg =
              "System: " + client->username + " joined the room\n";
          for (int j = 0; j < fds.size(); j++) {
            if (fds[j].fd == sock_fd || fds[j].fd == fds[i].fd)
              continue;

            Client *other = get_client(fds[j].fd, clients);
            if (other && other->room == client->room) {
              send(fds[j].fd, join_msg.c_str(), join_msg.size(), 0);
            }
          }

          // Confirm to user
          std::string confirm = "System: Joined room " + new_room + "\n";
          send(client->fd, confirm.c_str(), confirm.size(), 0);
          continue;
        }

        // /exit
        if (msg == "/exit") {
          if (client->room == "GLOBAL") {
            std::string err = "System: You are already in GLOBAL\n";
            send(client->fd, err.c_str(), err.size(), 0);
            continue;
          }

          std::cout << client->username << " returned to GLOBAL\n";

          // Notify old room
          std::string leave_msg =
              "System: " + client->username + " left the room\n";
          for (int j = 0; j < fds.size(); j++) {
            if (fds[j].fd == sock_fd || fds[j].fd == fds[i].fd)
              continue;

            Client *other = get_client(fds[j].fd, clients);
            if (other && other->room == client->room) {
              send(fds[j].fd, leave_msg.c_str(), leave_msg.size(), 0);
            }
          }

          client->room = "GLOBAL";

          // Notify GLOBAL
          std::string return_msg =
              "System: " + client->username + " returned to GLOBAL\n";
          for (int j = 0; j < fds.size(); j++) {
            if (fds[j].fd == sock_fd || fds[j].fd == fds[i].fd)
              continue;

            Client *other = get_client(fds[j].fd, clients);
            if (other && other->room == "GLOBAL") {
              send(fds[j].fd, return_msg.c_str(), return_msg.size(), 0);
            }
          }

          send(client->fd, "System: Returned to GLOBAL\n", 28, 0);
          continue;
        }

        // /online
        if (msg == "/online") {
          std::string list = "Online users in " + client->room + ":\n";
          int count = 0;
          for (auto &c : clients) {
            if (c.room == client->room) {
              list += "  👤 " + c.username + "\n";
              count++;
            }
          }
          if (count == 0) {
            list += "  No one else is here\n";
          }
          send(client->fd, list.c_str(), list.size(), 0);
          continue;
        }

        // /quit
        if (msg == "/quit") {
          std::string quit_msg = client->username + " quit\n";
          send(client->fd, quit_msg.c_str(), quit_msg.size(), 0);
          close(fds[i].fd);
          continue;
        }

        // ================= NORMAL MESSAGE =================
        if (!msg.empty()) {
          // Server log
          std::cout << "[" << client->room << "] " << client->username << ": "
                    << msg << "\n";

          // Broadcast to room (format: "username: message" without brackets)
          for (int j = 0; j < fds.size(); j++) {
            if (fds[j].fd == sock_fd || fds[j].fd == fds[i].fd)
              continue;

            Client *other = get_client(fds[j].fd, clients);
            if (other && other->room == client->room) {
              std::string out = client->username + ": " + msg + "\n";
              send(fds[j].fd, out.c_str(), out.size(), 0);
            }
          }
        }
      }
    }
  }

  close(sock_fd);
  return 0;
}
