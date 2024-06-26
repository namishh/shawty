#include "response.h"
#include "../server/server.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

const char *get_file_extension(const char *filepath) {
  const char *dot = strrchr(filepath, '.'); // Find the last occurrence of '.'
  if (!dot || dot == filepath)
    return ""; // No extension found or dot is the first character

  return dot + 1; // Return the extension (skip the dot character)
}

const char *get_mime_type(const char *file_ext) {
  if (strcasecmp(file_ext, "html") == 0 || strcasecmp(file_ext, "htm") == 0) {
    return "text/html";
  } else if (strcasecmp(file_ext, "txt") == 0) {
    return "text/plain";
  } else if (strcasecmp(file_ext, "css") == 0) {
    return "text/css";
  } else if (strcasecmp(file_ext, "js") == 0) {
    return "text/javascript";
  } else if (strcasecmp(file_ext, "jpg") == 0 ||
             strcasecmp(file_ext, "jpeg") == 0) {
    return "image/jpeg";
  } else if (strcasecmp(file_ext, "png") == 0) {
    return "image/png";
  } else if (strcasecmp(file_ext, "gif") == 0) {
    return "image/gif";
  } else if (strcasecmp(file_ext, "webp") == 0) {
    return "image/webp";
  } else {
    return "application/octet-stream";
  }
}

struct FileStats {
  char *content;
  size_t size;
};

struct FileStats get_file_stats(char *filename) {
  int file_fd = open(filename, O_RDONLY);
  char *response = (char *)malloc(BUFFER_SIZE * sizeof(char));
  size_t response_len = 0;

  struct stat file_stat;
  fstat(file_fd, &file_stat);

  // copy header to response buffer
  response_len = 0;

  // copy file to response buffer
  ssize_t bytes_read;
  while ((bytes_read = read(file_fd, response + response_len,
                            BUFFER_SIZE - response_len)) > 0) {
    response_len += bytes_read;
  }

  struct FileStats file_stats;
  file_stats.content = response;
  file_stats.size = response_len;
  return file_stats;
}

void render_static_file(struct Response *response, char *filename,
                        char *header) {
  char *status = "HTTP/1.1 200 OK\r\n";
  snprintf(header, BUFFER_SIZE, "%sContent-Type:%s\r\n\r\n", status,
           get_mime_type(get_file_extension(filename)));
  struct FileStats file_stats = get_file_stats(filename);

  size_t response_size = strlen(header) + file_stats.size;
  char *response_body = (char *)malloc(response_size * sizeof(char));

  memcpy(response_body, header, strlen(header));
  memcpy(response_body + strlen(header), file_stats.content, file_stats.size);

  response->status = status;
  response->body = response_body;
  response->size = response_size;
}

struct Response response_constructor(struct Request request) {
  char *header = (char *)malloc(BUFFER_SIZE * sizeof(char));
  struct Response response;
  if (strcmp(request.method, "GET") == 0) {
    if (strcmp(request.URI, "/") == 0) {
      render_static_file(&response, "./public/index.html", header);
    } else {
      render_static_file(&response, "./public/404.html", header);
    }
  } else if (strcmp(request.method, "POST") == 0) {
    printf("POST request\n");

  } else {
    render_static_file(&response, "./public/405.html", header);
  }
  return response;
}
