
#include <bits/sockaddr.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>

#define port 6767

bool fderrorChecker(int toCheck);
int main() {

  struct sockaddr addr;
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  if (fderrorChecker(fd)) {
    perror("Socket creation failed") exit(EXIT_FAILURE)
  }

  return 0;
}
bool fderrorChecker(int toCheck) {
  if (toCheck != -1) {
    return false;
  }
  return true;
}
