#ifndef DATABASE
#define DATABASE

#include <sqlite3.h>

struct URL {
  char *shortened;
  char *target;
};

struct Date {
  int day;
  int month;
  int year;
};

void seed_db();
void insert_url(char *target);
void check_if_shortened_exists(char *shortened);
char *get_target(char *shortened);

#endif // !DATABASE
