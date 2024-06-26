#include "request.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Header *head;

char *get_last_line(char *str) {
  char *last_newline = strrchr(str, '\n');
  if (last_newline != NULL) {
    return last_newline + 1;
  } else {
    return str;
  }
}

void empty_headers() {
  struct Header *temp = head;
  struct Header *next;
  while (temp != NULL) {
    next = temp->next;
    free(temp);
    temp = next;
  }
  head = NULL;
}

void request_add_header(char *string) {
  struct Header *temp = (struct Header *)malloc(sizeof(struct Header));
  temp->string = strdup(string);
  temp->next = NULL;
  if (head == NULL) {
    head = temp;
    return;
  } else {
    struct Header *temp2 = head;
    while (temp2->next != NULL) {
      temp2 = temp2->next;
    }
    temp2->next = temp;
  }
}

// I won't really need the headers for this so Im just parsing them line by line
// for the formality and not as in a key: value kind of thing.

int is_only_whitespace_or_escape(char *string) {
  for (int i = 0; i < strlen(string); i++) {
    if (!isspace(string[i])) {
      return 0;
    }
  }
  return 1;
}

void parse_headers(char *header_fields) {
  empty_headers();
  char fields[strlen(header_fields)];
  strcpy(fields, header_fields);
  char *field = strtok(fields, "\n");
  // First parse out the wholes lines
  while (field) {
    if (is_only_whitespace_or_escape(field)) {
      break;
    }
    request_add_header(field);
    printf("Field: %s\n", field);
    field = strtok(NULL, "\n");
  }
}

void print_headers() {
  struct Header *temp = head;
  printf("===========HeaderStrings==========\n");
  while (temp != NULL) {
    printf("%s\n", temp->string);
    temp = temp->next;
  }
  printf("==================================\n");
}

struct Request request_constructor(char *string) {
  // seperate data and body
  for (int i = 0; i < strlen(string) - 1; i++) {
    if (string[i] == '\n' && string[i + 1] == '\n') {
      string[i + 1] = '|';
    }
  }
  char *dup = strdup(string);
  char *request_line = strtok(string, "\n");
  char *header_fields =
      strtok(NULL, "|"); // NULL means continue using the line we were using
  struct Request request;
  char *body = "";

  char *method = strtok(request_line, " ");
  char *URI = strtok(NULL, " ");
  char *HTTPVersion = strtok(NULL, " ");
  HTTPVersion = strtok(HTTPVersion, "/");
  HTTPVersion = strtok(NULL, "/");

  request.method = method;
  request.URI = URI;
  request.HTTPVersion = (float)atof(HTTPVersion);

  parse_headers(header_fields);

  if (strcmp(method, "GET") != 0) {
    body = get_last_line(dup);
  }

  request.body = body;
  // print_headers();
  return request;
}
