#include <arpa/inet.h>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <iterator>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
using namespace std;

// int getaddrinfo(const char *node, const char *service,
//                 const struct addrinfo *hints, struct addrinfo **res);
// int status;
// struct addrinfo hints;
// struct addrinfo *servinfo;
//
// hints.ai_family = AF_UNSPEC

/*struct addrinfo {
int ai_flags; // AI_PASSIVE, AI_CANONNAME, etc.
int ai_family; // AF_INET, AF_INET6, AF_UNSPEC
int ai_socktype; // SOCK_STREAM, SOCK_DGRAM
int ai_protocol; // use 0 for "any"
size_t ai_addrlen; // size of ai_addr in bytes
struct sockaddr *ai_addr; // struct sockaddr_in or _in6
char *ai_canonname; // full canonical hostname
struct addrinfo *ai_next; // linked list, next node
};*/

/*struct sockaddr {
char unsigned short sa_family; // address family, AF_xxx
sa_data[14]; // 14 bytes of protocol address
};*/

// (IPv4 only--see struct sockaddr_in6 for IPv6)
// struct sockaddr_in {
// short int sin_family; // Address family, AF_INET
// unsigned short int sin_port; // Port number
// struct in_addr sin_addr; // Internet address
// unsigned char sin_zero[8]; // Same size as struct sockaddr
// };
struct sockaddr_ina6;
int main() {
  // addrinfo is used to specify the ipv and It’s also used in host name
  // lookups, and service name lookups.
  cout << addrinfo().ai_family;
  cout << endl;
  cout << addrinfo().ai_protocol;
  cout << endl;
  cout << addrinfo().ai_next;
  cout << endl;
  cout << sockaddr().sa_family;
  cout << endl;
  cout << sockaddr().sa_len;
  cout << endl;
  // sockaddr consist all the details of sockets;
  // inet_pton is used to convert the string ip address into its binary
  // equivalent
  // ntop is opposite of ptop
  // ntop stands for numeber(binary in this case to presentation);
  //  inet_ntop(AF_INET6, const void *, char *, socklen_t size);
  char ip6[INET6_ADDRSTRLEN];
  struct sockaddr_in6 sa6;
  inet_ntop(AF_INET6, &sa6.sin6_addr, ip6, INET6_ADDRSTRLEN);
  cout << "The address is :" << ip6;
  cout << endl;
  int status;
  struct addrinfo hints;
  struct addrinfo *servinfo; // will point to the results

  memset(&hints, NULL, sizeof hints);
  hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

  // get ready to connect
  status = getaddrinfo("www.example.net", "3490", &hints, &servinfo);

  // servinfo now points to a linked list of 1 or more
  // struct addrinfos

  // etc.
  // service is used for what internet protocol is being used like http or it
  // can also be used as a port number ;
}
