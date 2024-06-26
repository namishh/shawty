#include "response.h"
#include "../database/database.h"
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

void parse_url(const char *str, char **url) {
  char *token;
  char *temp_str = strdup(str);

  // Get the URL
  token = strtok(temp_str, "=&");
  if (token != NULL && strcmp(token, "url") == 0) {
    token = strtok(NULL, "=&");
    if (token != NULL) {
      *url = strdup(token);
    }
  }

  free(temp_str);
}

void decode_url(char *str) {
  char *p = str;
  int value;

  while (*p) {
    if (*p == '%') {
      sscanf(p + 1, "%2x", &value);
      *p = (char)value;
      memmove(p + 1, p + 3, strlen(p + 3) + 1);
    }
    p++;
  }
}

char *get_substring_after_route(char *str) {
  char *route = "/s/";
  char *found = strstr(str, route);

  if (found != NULL) {
    return found + strlen(route);
  } else {
    return NULL;
  }
}

void render_json(struct Response *response, char *url, char *header) {
  char *status = "HTTP/1.1 200 OK\r\n";
  snprintf(header, BUFFER_SIZE, "%sContent-Type: application/json\r\n\r\n",
           status);

  char final_string[1000];
  sprintf(final_string, "{\"url\": \"%s\"}", url);

  size_t response_size = strlen(header) + strlen(final_string);
  char *response_body = (char *)malloc(response_size * sizeof(char));

  memcpy(response_body, header, strlen(header));
  memcpy(response_body + strlen(header), final_string, strlen(final_string));

  response->status = status;
  response->body = response_body;
  response->size = response_size;
}

int starts_with(const char *str, const char *prefix) {
  size_t len_prefix = strlen(prefix);
  return strncmp(str, prefix, len_prefix) == 0;
}

void render_html_element(struct Response *response, char *header,
                         char *template) {
  char *status = "HTTP/1.1 200 OK\r\n";
  snprintf(header, BUFFER_SIZE, "%sContent-Type: text/html\r\n\r\n", status);

  char final_string[10000];
  sprintf(final_string, "%s", template);

  size_t response_size = strlen(header) + strlen(final_string);
  char *response_body = (char *)malloc(response_size * sizeof(char));

  memcpy(response_body, header, strlen(header));
  memcpy(response_body + strlen(header), final_string, strlen(final_string));

  response->status = status;
  response->body = response_body;
  response->size = response_size;
}

void reroute(struct Response *response, char *header, char *target) {
  char *status = "HTTP/1.1 301 Moved Permanently\r\n";
  snprintf(header, BUFFER_SIZE, "%sLocation: %s\r\n\r\n", status, target);

  size_t response_size = strlen(header);
  char *response_body = (char *)malloc(response_size * sizeof(char));

  memcpy(response_body, header, strlen(header));

  response->status = status;
  response->body = response_body;
  response->size = response_size;
}

struct Response response_constructor(struct Request request) {
  char *header = (char *)malloc(BUFFER_SIZE * sizeof(char));
  struct Response response;
  if (strcmp(request.method, "GET") == 0) {
    if (strcmp(request.URI, "/") == 0 ||
        strstr(request.URI, "/?url=") != NULL) {
      render_static_file(&response, "./public/index.html", header);
    } else if (strstr(request.URI, "/s/") != NULL) {
      char *shortened = get_substring_after_route(request.URI);
      struct URL url = get_url(shortened);
      if (strcmp(url.target, "") != 0) {
        reroute(&response, header, url.target);
      } else {
        render_static_file(&response, "./public/404_route.html", header);
      }
    } else if (strcmp(request.URI, "/links") == 0) {
      char *template = generate_html_list();
      printf("TEMPLATE\n%s\n", template);
      render_html_element(&response, header, template);
    } else {
      render_static_file(&response, "./public/404.html", header);
    }
  } else if (strcmp(request.method, "POST") == 0) {
    char *url;

    parse_url(request.body, &url);
    decode_url(url);

    printf("%s", url);

    char *message;
    char *color;

    if (strcmp(url, "") == 0) {
      message = "No URL provided";
      color = "red-400";
    } else {
      if (starts_with(url, "https://") == 0) {
        message = "URL is invalid";
        color = "red-400";
      } else {
        message = "URL Shortened";
        color = "green-400";
        insert_url(url);
      }
    }

    char template[1000];
    sprintf(template, "<p class='text-%s mx-4 mt-2'>%s</p>", color, message);

    render_html_element(&response, header, template);
  } else if (strcmp(request.method, "DELETE") == 0) {
    if (strcmp(request.URI, "/links") == 0) {
      delete_all_urls();
      char template[] =
          "<p class='text-red-400 mx-4 mt-2'>All URLs deleted</p>";
      render_html_element(&response, header, template);
    }
  } else {
    render_static_file(&response, "./public/405.html", header);
  }
  return response;
}
