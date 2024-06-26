#ifndef REQUEST
#define REQUEST

struct Request {
  char *method;
  char *URI;
  float HTTPVersion;
  char *body;
  struct Header *request_headers_head;
};

struct Header {
  char *string;
  struct Header *next;
};

struct Request request_constructor(char *string);
void request_add_header(char *string);
void print_headers();

#endif // REQUEST
