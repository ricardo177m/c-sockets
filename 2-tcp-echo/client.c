#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define HOST "localhost"
#define PORT "1337"
#define BUFFER_SIZE 100

int main() {
  struct addrinfo req, *list;

  memset(&req, 0, sizeof(req));

  req.ai_family = AF_UNSPEC;     // determine which one is used with getaddrinfo
  req.ai_socktype = SOCK_STREAM; // tcp

  if (getaddrinfo(HOST, PORT, &req, &list) != 0) {
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

  // connect
  if (connect(sockfd, list->ai_addr, list->ai_addrlen) < 0) {
    perror("connect failed");
    freeaddrinfo(list);
    close(sockfd);
    return 1;
  }

  freeaddrinfo(list);

  puts("Connection established!");

  char buffer[BUFFER_SIZE];

  while (1) {
    printf("Write a message to send: ");
    fgets(buffer, sizeof(buffer), stdin);

    // remove new line
    int msg_length = strlen(buffer);
    if (buffer[msg_length - 1] == '\n') {
      buffer[msg_length - 1] = 0;
      msg_length--;
    }

    write(sockfd, &buffer, msg_length + 1);

    printf("Sent!\n");

    int read_len = read(sockfd, &buffer, BUFFER_SIZE);

    if (read_len <= 0) {
      perror("error reading from server");
      close(sockfd);
      return 1;
    }

    printf("Read: %s\n", buffer);
  }

  close(sockfd);

  return 0;
}
