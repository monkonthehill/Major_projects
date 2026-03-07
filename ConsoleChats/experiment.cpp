#include <arpa/inet.h> // inet_ntop(), inet_pton()
#include <cstring>     // memset()
#include <iostream>
#include <netdb.h>      // addrinfo, getaddrinfo()
#include <netinet/in.h> // sockaddr_in, sockaddr_in6
#include <sys/socket.h> // socket structures
#include <sys/types.h>

using namespace std;

int main() {

  /*
  ---------------------------------------------------------
  BASIC LOOK AT SOME SOCKET STRUCTURES
  ---------------------------------------------------------
  addrinfo:
  Used by getaddrinfo() to describe network addresses.
  It helps resolve hostnames and services into socket
  addresses usable by socket(), connect(), bind(), etc.
  */

  struct addrinfo example;

  cout << "addrinfo.ai_family: " << example.ai_family << endl;
  cout << "addrinfo.ai_protocol: " << example.ai_protocol << endl;
  cout << "addrinfo.ai_next: " << example.ai_next << endl;

  /*
  ---------------------------------------------------------
  sockaddr STRUCTURE
  ---------------------------------------------------------
  Generic socket address structure used internally by the OS.
  It stores the address family and raw address data.
  */

  struct sockaddr generic_addr;

  cout << "sockaddr.sa_family: " << generic_addr.sa_family << endl;

  /*
  ---------------------------------------------------------
  inet_ntop() DEMONSTRATION
  ---------------------------------------------------------
  inet_ntop() converts a binary IP address into a human
  readable string.

  ntop = Network → Presentation
  */

  char ip6[INET6_ADDRSTRLEN]; // buffer to store IPv6 string
  struct sockaddr_in6 sa6;    // IPv6 socket structure

  inet_ntop(AF_INET6, &sa6.sin6_addr, ip6, INET6_ADDRSTRLEN);

  cout << "The address is: " << ip6 << endl;

  /*
  ---------------------------------------------------------
  getaddrinfo()
  ---------------------------------------------------------
  Used to resolve a hostname + service (port) into usable
  socket address structures.
  */

  int status;
  struct addrinfo hints;
  struct addrinfo *servinfo; // will point to the result list

  // Clear the hints structure before using it
  memset(&hints, 0, sizeof hints);

  // Allow IPv4 or IPv6
  hints.ai_family = AF_UNSPEC;

  // Request TCP sockets
  hints.ai_socktype = SOCK_STREAM;

  /*
  node    → hostname or IP
  service → port number or service name
  */

  status = getaddrinfo("www.example.net", "3490", &hints, &servinfo);

  /*
  If successful:
  servinfo will point to a linked list of addrinfo structures
  that contain usable socket addresses.
  */

  if (status != 0) {
    cout << "getaddrinfo error: " << gai_strerror(status) << endl;
    return 1;
  }

  cout << "Address lookup successful." << endl;

  // Always free memory allocated by getaddrinfo()
  freeaddrinfo(servinfo);

  return 0;
}
