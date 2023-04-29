#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "1337"
#define BUFFER_SIZE 1024

int main() {
  /**
   * In C, each socket belongs to a certain address family specified in the
   * domain argument of the socket function, AF_INET for IPv4 & AF_INET6 for
   * IPv6. An AF_INET address family socket uses only the IPv4 stack, whereas an
   * AF_INET6 socket uses both IPv4 & IPv6 stacks - however, an AF_INET socket
   * can only handle IPv6 addresses. By using IPv4-mapped addresses, which are a
   * way to represent IPv4 addresses in the IPv6 format, an AF_INET6 is able to
   * communicate using IPv4 in dual-stack nodes.
   *
   * The typical socket types used in applications are datagram sockets for UDP
   * & stream sockets for TCP.
   */

  // address info struct
  struct addrinfo req, *list;

  memset(&req, 0, sizeof(req));

  req.ai_family = AF_INET6;     // ipv6
  req.ai_socktype = SOCK_DGRAM; // udp
  req.ai_flags = AI_PASSIVE;    // wildcard ip addresses

  // translate hostnames or ip addresses in human readable format to a
  // structured binary format address that can be used to bind
  // null node for local host
  if (getaddrinfo(NULL, PORT, &req, &list) < 0) {
    perror("unable to translate to an address");
    return 1;
  }

  // create socket
  int sockfd = socket(list->ai_family, list->ai_socktype, list->ai_protocol);

  if (sockfd < 0) {
    perror("failed to create the socket");
    freeaddrinfo(list);
    return 1;
  }

  // bind
  if (bind(sockfd, list->ai_addr, list->ai_addrlen) < 0) {
    perror("bind error");
    freeaddrinfo(list);
    close(sockfd);
    return 1;
  }

  freeaddrinfo(list);

  // udp has the possibility of sending datagrams to broadcast (ipv4) or
  // multicast (ipv4 + ipv6) addresses; to enable, set the socket option
  // SO_BROADCAST to 1
  // int val = 1;
  // if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &val, sizeof(val)) < 0) {
  // perror("failed setting socket options");
  // return 1;
  // }

  printf("Ready to receive requests on port %s!\n", PORT);

  char buffer[BUFFER_SIZE];

  // struct used to save the source address and port for replying
  // this struct sockaddr_storage is as large as any other sockaddr_* and it's
  // aligned so that a pointer to it can be cast as a pointer to other
  // sockaddr_* structs
  struct sockaddr_storage src_addr;
  socklen_t src_addr_len = sizeof(src_addr);

  while (1) {
    // receive request
    ssize_t msglen = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                              (struct sockaddr *)&src_addr, &src_addr_len);

    if (msglen < 0) {
      perror("error receiving udp datagram");
      close(sockfd);
      return 1;
    }

    // print source address
    char readable_addr[50], port[6];
    getnameinfo((struct sockaddr *)&src_addr, src_addr_len, readable_addr,
                sizeof(readable_addr), port, sizeof(port),
                NI_NUMERICHOST | NI_NUMERICSERV);

    printf("Received request from %s port %s: %s\n", readable_addr, port,
           buffer);

    // send reply
    char *msg = "Hello world!";
    if (sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&src_addr,
               src_addr_len) < 0) {
      perror("error sending udp datagram");
      close(sockfd);
      return 1;
    }

    printf("Sent: %s\n", msg);
  }

  // cleanup
  close(sockfd);

  return 0;
}
