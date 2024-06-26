#include "database/database.h"
#include "request/request.h"
#include "response/response.h"
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

    struct Request request = request_constructor(buffer);

    printf("Method: %s\n", request.method);
    printf("URI: %s\n", request.URI);
    printf("Body: %s\n", request.body);

    struct Response response = response_constructor(request);

    send(new_socket, response.body, response.size, 0);

    // close the new_socket
    close(new_socket);
  }
}

int main() {
  seed_db();
  struct Server server =
      server_constructor(AF_INET, SOCK_STREAM, 0, PORT, 3, INADDR_ANY, start);

  server.start(&server);

  return 0;
}
