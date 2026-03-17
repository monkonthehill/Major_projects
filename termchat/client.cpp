#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <unistd.h>

using namespace std;

void receive_messages(int sock) {
  char buffer[1024];

  while (true) {
    memset(buffer, 0, sizeof(buffer));

    int bytes = read(sock, buffer, sizeof(buffer));

    if (bytes <= 0) {
      cout << "\nServer disconnected\n";
      exit(0);
    }

    cout << "\nServer: " << buffer << endl;
    cout << "You: " << flush;
  }
}

int main() {
  int sock;
  sockaddr_in serv_addr;
  char buffer[1024];

  sock = socket(AF_INET, SOCK_STREAM, 0);

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(8080);
  inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

  connect(sock, (sockaddr *)&serv_addr, sizeof(serv_addr));

  // start receiving thread
  thread t(receive_messages, sock);

  while (true) {
    cout << "You: ";
    cin.getline(buffer, sizeof(buffer));

    send(sock, buffer, strlen(buffer), 0);
  }

  t.join();
  close(sock);
}
