#include "server/server.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void start(struct Server *server) {
  char buffer[BUFFER_SIZE];
  int new_socket;
  int addrlen = sizeof(server->address);

  while (1) {
    printf("Waitng for connections...\n");
    // accept() -> accepts a connection on a socket.
    new_socket = accept(server->sock, (struct sockaddr *)&server->address,
                        (socklen_t *)&addrlen);
    // read() -> read from a file descriptor
    memset(buffer, 0, BUFFER_SIZE * sizeof(char));
    read(new_socket, buffer, BUFFER_SIZE);

    printf("===========BUFFER========\n");
    printf("%s\n", buffer);
    printf("=========================\n");

    send(new_socket, "", 1024, 0);

    // close the new_socket
    close(new_socket);
  }
}

int main() {
  struct Server server =
      server_constructor(AF_INET, SOCK_STREAM, 0, PORT, 3, INADDR_ANY, start);

  server.start(&server);

  return 0;
}
