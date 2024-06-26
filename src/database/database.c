#include "database.h"
#include <time.h>

sqlite3 *DB;

char *DB_FILE = "data.db";

struct Date current_date() {
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  struct Date date = {tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900};

  return date;
}

void seed_db() {
  struct Date date = current_date();
  char *err_msg = 0;
  int rc = sqlite3_open(DB_FILE, &DB);

  if (rc != SQLITE_OK) {
    sqlite3_close(DB);
  }

  char *sql = "CREATE TABLE IF NOT EXISTS urls ("
              "shortened TEXT PRIMARY KEY NOT NULL,"
              "day INT NOT NULL,"
              "month INT NOT NULL,"
              "year INT NOT NULL,"
              "expiry TEXT NOT NULL,"
              "target TEXT NOT NULL);";

  rc = sqlite3_exec(DB, sql, 0, 0, &err_msg);

  if (rc != SQLITE_OK) {
    sqlite3_free(err_msg);
    sqlite3_close(DB);
  }

  sqlite3_close(DB);
}
