#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <unistd.h>

using namespace std;

volatile bool running = true;

void receive_messages(int sock) {
  char buffer[1024];

  while (running) {
    memset(buffer, 0, sizeof(buffer));

    int bytes = read(sock, buffer, sizeof(buffer) - 1);

    if (bytes <= 0) {
      cout << "\n[Disconnected from server]\n";
      running = false;
      break;
    }

    cout << "\rServer: " << buffer << endl;
    cout << "You: " << flush;
  }
}

int main() {
  int sock;
  sockaddr_in serv_addr{};
  char buffer[1024];

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    perror("socket");
    return 1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(8080);

  if (inet_pton(AF_INET, "10.48.101.96", &serv_addr.sin_addr) <= 0) {
    perror("inet_pton");
    return 1;
  }

  if (connect(sock, (sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
    perror("connect");
    return 1;
  }

  cout << "[Connected to server]\n";

  thread receiver(receive_messages, sock);

  while (running) {
    cout << "You: ";
    cin.getline(buffer, sizeof(buffer));

    // handle quit
    if (strcmp(buffer, "/quit") == 0) {
      running = false;
      break;
    }

    if (strlen(buffer) == 0)
      continue;

    if (send(sock, buffer, strlen(buffer), 0) == -1) {
      perror("send");
      break;
    }
  }

  close(sock);
  receiver.join();

  cout << "[Client exited]\n";
  return 0;
}
