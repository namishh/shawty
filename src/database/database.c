#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

sqlite3 *DB;

char *DB_FILE = "data.db";

void seed_db() {
  char *err_msg = 0;
  int rc = sqlite3_open(DB_FILE, &DB);

  if (rc != SQLITE_OK) {
    sqlite3_close(DB);
  }

  char *sql = "CREATE TABLE IF NOT EXISTS urls ("
              "shortened TEXT PRIMARY KEY NOT NULL,"
              "target TEXT NOT NULL);";

  rc = sqlite3_exec(DB, sql, 0, 0, &err_msg);

  if (rc != SQLITE_OK) {
    sqlite3_free(err_msg);
    sqlite3_close(DB);
  }

  sqlite3_close(DB);
}

int check_if_shortened_exists(char *shortened) {
  char *err_msg = 0;
  int rc = sqlite3_open(DB_FILE, &DB);

  if (rc != SQLITE_OK) {
    sqlite3_close(DB);
  }

  char *sql =
      sqlite3_mprintf("SELECT * FROM urls WHERE shortened = '%s';", shortened);

  rc = sqlite3_exec(DB, sql, 0, 0, &err_msg);

  if (rc != SQLITE_OK) {
    sqlite3_free(err_msg);
    sqlite3_close(DB);
    return 0;
  }

  sqlite3_close(DB);
  return 1;
}

void generate_random_string(char *str, size_t size) {
  // initialse
  srand(time(NULL));
  const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0"
                         "1234567899876543210";
  for (size_t i = 0; i < size; i++) {
    size_t key = rand() % (sizeof(charset) - 1);
    str[i] = charset[key];
  }
  str[size] = '\0';
}

void insert_url(char *target) {
  char *err_msg = 0;
  int rc = sqlite3_open(DB_FILE, &DB);

  if (rc != SQLITE_OK) {
    sqlite3_close(DB);
  }
  char shortened[9];

  // generate random strings until it is unique
  generate_random_string(shortened, 8);

  char *sql = sqlite3_mprintf("INSERT INTO urls (shortened, target) "
                              "VALUES ('%s', '%s');",
                              shortened, target);

  rc = sqlite3_exec(DB, sql, 0, 0, &err_msg);

  if (rc != SQLITE_OK) {
    printf("Error: %s\n", err_msg);
    sqlite3_free(err_msg);
    sqlite3_close(DB);
  }

  sqlite3_close(DB);
}

struct URL get_url(char *shortened) {
  char *err_msg = 0;
  int rc = sqlite3_open(DB_FILE, &DB);

  if (rc != SQLITE_OK) {
    sqlite3_close(DB);
  }

  char *sql =
      sqlite3_mprintf("SELECT * FROM urls WHERE shortened = '%s';", shortened);

  rc = sqlite3_exec(DB, sql, 0, 0, &err_msg);

  if (rc != SQLITE_OK) {
    sqlite3_free(err_msg);
    sqlite3_close(DB);
  }

  struct URL url;
  sqlite3_stmt *res;
  sqlite3_prepare_v2(DB, sql, -1, &res, 0);
  // check if the result is empty
  if (sqlite3_step(res) != SQLITE_ROW) {
    sqlite3_close(DB);
    url.shortened = "";
    url.target = "";
    return url;
  }
  url.shortened = (char *)sqlite3_column_text(res, 0);
  url.target = (char *)sqlite3_column_text(res, 1);

  sqlite3_close(DB);
  return url;
}

void delete_url(char *shortened) {
  char *err_msg = 0;
  int rc = sqlite3_open(DB_FILE, &DB);

  if (rc != SQLITE_OK) {
    sqlite3_close(DB);
  }

  char *sql =
      sqlite3_mprintf("DELETE FROM urls WHERE shortened = '%s';", shortened);

  rc = sqlite3_exec(DB, sql, 0, 0, &err_msg);

  if (rc != SQLITE_OK) {
    sqlite3_free(err_msg);
    sqlite3_close(DB);
  }

  sqlite3_close(DB);
}

// generate a list of shortened urls
char *generate_html_list() {
  char *template = malloc(12000 * sizeof(char));
  char *list = malloc(10000 * sizeof(char));
  memset(template, 0, 12000 * sizeof(char));
  memset(list, 0, 10000 * sizeof(char));

  // get all the urls
  char *err_msg = 0;
  int rc = sqlite3_open(DB_FILE, &DB);
  char *sql = "SELECT * FROM urls;";
  rc = sqlite3_exec(DB, sql, 0, 0, &err_msg);

  if (rc != SQLITE_OK) {
    sqlite3_free(err_msg);
    sqlite3_close(DB);
  }

  sqlite3_stmt *res;
  sqlite3_prepare_v2(DB, sql, -1, &res, 0);

  // check if the result is empty
  if (sqlite3_step(res) != SQLITE_ROW) {
    sqlite3_close(DB);
    strcat(
        list,
        "<h1 class='text-3xl text-white font-bold'>Start generating URLs!<h1>");
    sprintf(template, "<div class='p-4'>%s</div>", list);
    return template;
  }

  sqlite3_reset(res);

  // loop through the results
  while (1) {
    int rc = sqlite3_step(res);
    if (rc == SQLITE_DONE) {
      break;
    } else if (rc != SQLITE_ROW) {
      sqlite3_close(DB);
      return NULL;
    }
    char *shortened = (char *)sqlite3_column_text(res, 0);
    char *target = (char *)sqlite3_column_text(res, 1);
    char webtarget[40];
    sprintf(webtarget, "http://localhost:6969/s/%s", shortened);
    char tag[256];
    sprintf(
        tag,
        "<li class='bg-gray-900/70 text-white p-3 my-2 "
        "rounded-xl'><details><summary>%s | %s</"
        "summary> <a href='%s' class='text-blue-400 my-2'>%s</a></details><li>",
        shortened, target, webtarget, webtarget);

    strcat(list, tag);
  }

  sprintf(template, "<ul class='p-4'>%s</ul>", list);

  return template;
}

void delete_all_urls() {
  char *err_msg = 0;
  int rc = sqlite3_open(DB_FILE, &DB);

  if (rc != SQLITE_OK) {
    sqlite3_close(DB);
  }

  char *sql = "DELETE FROM urls;";

  rc = sqlite3_exec(DB, sql, 0, 0, &err_msg);

  if (rc != SQLITE_OK) {
    sqlite3_free(err_msg);
    sqlite3_close(DB);
  }

  sqlite3_close(DB);
}
