#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define HOST "localhost"
#define PORT "1337"

int main() {
  // address info struct
  struct addrinfo req, *serverList, *localList;

  memset(&req, 0, sizeof(req));

  // use of ipv4 or ipv6 depends on the address of the server to be reached
  // first use getaddrinfo to process the server's address
  req.ai_family = AF_UNSPEC;    // ipv4 or ipv6
  req.ai_socktype = SOCK_DGRAM; // udp

  // translate hostnames or ip addresses in human readable format to a
  // structured binary format address that can be used to bind
  if (getaddrinfo(HOST, PORT, &req, &serverList) < 0) {
    perror("unable to translate to an address");
    return 1;
  }

  // now we can create a conforming socket
  memset(&req, 0, sizeof(req));

  req.ai_family = serverList->ai_family; // use the same family
  req.ai_socktype = SOCK_DGRAM;          // udp
  req.ai_flags = AI_PASSIVE;             // wildcard ip addresses

  // translate hostnames or ip addresses in human readable format to a
  // structured binary format address that can be used to bind
  // null node for local host, port 0 for auto assigning on bind
  if (getaddrinfo(NULL, "0", &req, &localList) < 0) {
    perror("unable to translate to an address");
    freeaddrinfo(serverList);
    return 1;
  }

  // create socket
  int sockfd = socket(localList->ai_family, localList->ai_socktype,
                      localList->ai_protocol);

  if (sockfd < 0) {
    perror("failed to create the socket");
    freeaddrinfo(serverList);
    freeaddrinfo(localList);
    return 1;
  }

  // bind
  if (bind(sockfd, localList->ai_addr, localList->ai_addrlen) < 0) {
    perror("bind error");
    freeaddrinfo(serverList);
    freeaddrinfo(localList);
    close(sockfd);
    return 1;
  }

  freeaddrinfo(localList);

  // set a receive timeout option so that the process doesn't get blocked
  // forever
  struct timeval to = {0};
  to.tv_sec = 10;
  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&to, sizeof(to)) <
      0) {
    perror("failed setting socket options");
    freeaddrinfo(serverList);
    close(sockfd);
    return 1;
  }

  // make the request
  char *msg = "request!";
  if (sendto(sockfd, msg, strlen(msg) + 1, 0, serverList->ai_addr,
             serverList->ai_addrlen) < 0) {
    perror("error sending the request");
    freeaddrinfo(serverList);
    close(sockfd);
    return 1;
  }

  printf("Sent: %s\n", msg);

  // receive the reply
  char buffer[1024];
  if (recv(sockfd, buffer, sizeof(buffer), 0) < 0) {
    perror("error receiving udp datagram");
    freeaddrinfo(serverList);
    close(sockfd);
    return 1;
  }

  printf("Received: %s\n", buffer);

  // cleanup
  freeaddrinfo(serverList);
  close(sockfd);

  return 0;
}
