
#include <bits/sockaddr.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define port 6767

bool errorChecker(int toCheck);

int main() {

  int socketFd = socket(AF_INET, SOCK_STREAM, 0);

  if (errorChecker(socketFd)) {
    perror("Socket creation failed\n");
    exit(EXIT_FAILURE);
  }

  printf("socket Created succesfully\n");

  struct sockaddr_in addres;

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

  for (;;) {
  }

  return 0;
}
bool errorChecker(int toCheck) {
  if (toCheck != -1) {
    return false;
  }
  return true;
}
