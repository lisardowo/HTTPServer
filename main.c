
#include <bits/sockaddr.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define port 6769
char buffer[1024];
const char serverResponse[] = "HTTP/1.1 200 OK\r\n"
                              "Content-Type: text/html; charset=UTF-8\r\n"
                              "Content-Length: 53\r\n"
                              "Connection: close\r\n"
                              "\r\n"
                              "<!DOCTYPE html><html><body><h1>chinga tu madre "
                              "nigga siseven</h1></body></html>";

bool errorChecker(int toCheck);

int main() {

  struct sockaddr_in addres;

  int socketFd = socket(AF_INET, SOCK_STREAM, 0);
  // socklen_t addrlen = sizeof(addres);

  if (errorChecker(socketFd)) {
    perror("Socket creation failed\n");
    exit(EXIT_FAILURE);
  }

  printf("socket Created succesfully\n");

  int opt = 1;
  setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  addres.sin_family = AF_INET;
  addres.sin_port = htons(port);
  addres.sin_addr.s_addr = INADDR_ANY;
  memset(&addres.sin_zero, 0, sizeof(addres.sin_zero));

  int bindSucceed = bind(socketFd, (struct sockaddr *)&addres, sizeof(addres));

  if (errorChecker(bindSucceed)) {
    perror("Error binding to socket\n");
    exit(EXIT_FAILURE);
  }

  printf("Socket binded at port %d\n", port);

  listen(socketFd, 10);

  printf("server listening at %d\n", port);

  int clientFd = -1;

  for (;;) {
    struct sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);

    int clientFd = accept(socketFd, (struct sockaddr *)&addres, &addrlen);

    if (errorChecker(clientFd)) {
      perror("accept failed\n");
      continue;
    }

    memset(buffer, 0, sizeof(buffer));
    ssize_t receivedPayload = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

    if (receivedPayload > 0) {
      printf(" === Received Request ===\n%s\n", buffer);

      send(clientFd, serverResponse, strlen(serverResponse), 0);
    }
    close(clientFd);
    clientFd = -1;
  }

  printf("closing connections/servers\n");

  if (clientFd >= 0) {
    close(clientFd);
  }
  close(socketFd);

  return 0;
}
bool errorChecker(int toCheck) {
  if (toCheck != -1) {
    return false;
  }
  return true;
}
