#ifndef RESPONSE_H
#define RESPONSE_H
#include "../request/request.h"
#include <stdlib.h>
#include <string.h>

struct Response {
  char *status;
  char *body;
  size_t size;
};

struct Response response_constructor(struct Request request);

#endif // RESPONSE_H
