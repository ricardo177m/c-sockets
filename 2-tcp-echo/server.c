#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "1337"

int main() {
  struct addrinfo req, *list;

  memset(&req, 0, sizeof(req));

  req.ai_family = AF_INET6;      // ipv6
  req.ai_socktype = SOCK_STREAM; // tcp
  req.ai_flags = AI_PASSIVE;     // accept connections

  if (getaddrinfo(NULL, PORT, &req, &list) != 0) {
    perror("getaddrinfo failed");
    return 1;
  }

  // create socket
  int sockfd = socket(list->ai_family, list->ai_socktype, list->ai_protocol);

  if (sockfd < 0) {
    perror("failed creating socket");
    freeaddrinfo(list);
    return 1;
  }

  // bind
  if (bind(sockfd, list->ai_addr, list->ai_addrlen) < 0) {
    perror("bind failed");
    freeaddrinfo(list);
    close(sockfd);
    return 1;
  }

  freeaddrinfo(list);

  // listen
  // SOMAXCONN is the highest supported value for the number of
  // pending connections
  if (listen(sockfd, SOMAXCONN) < 0) {
    perror("listen failed");
    close(sockfd);
    return 1;
  }

  printf("Server listening on port %s!\n", PORT);

  // a tcp connection is only established after it being accepted by the server
  // in case of success, accept() will return a socket connected to the client

  // struct used to save the source address and port
  struct sockaddr_storage src_addr;
  socklen_t src_addr_len = sizeof(src_addr);

  while (1) {
    int client_sock =
        accept(sockfd, (struct sockaddr *)&src_addr, &src_addr_len);

    // events on tcp sockets are asynchronous
    // we can use another process to handle each client socket
    pid_t p = fork();

    if (p < 0) {
      perror("fork failed");
      close(sockfd);
      return 1;
    }

    if (p == 0) {
      close(sockfd);

      // print source address
      char readable_addr[50], port[6];
      getnameinfo((struct sockaddr *)&src_addr, src_addr_len, readable_addr,
                  sizeof(readable_addr), port, sizeof(port),
                  NI_NUMERICHOST | NI_NUMERICSERV);

      printf("Connected to %s port %s!\n", readable_addr, port);

      // in udp applications, each send must match one receive in the
      // counterpart and the number of bytes transported by each datagram is
      // not required to be known prior to the reception; however, in tcp
      // applications, the number of bytes being read must exactly match the
      // number of bytes written in the counterpart, meaning that it is
      // required to apply a protocol design to ensure applications always
      // know exactly how many bytes they are supposed to read

      // ...

      char buffer[100];

      while (1) {
        int received_length = recv(client_sock, buffer, sizeof(buffer), 0);

        if (received_length == 0) {
          puts("Bye bye!");
          close(client_sock);
          exit(0);
        }

        if (received_length < 0) {
          perror("error reading from client socket");
          close(client_sock);
          exit(1);
        }

        printf("Received %d bytes: %s\n", received_length, buffer);

        write(client_sock, buffer, received_length);

        puts("Echoed!");
      }
    }

    close(client_sock);
  }

  // cleanup
  close(sockfd);

  return 0;
}
