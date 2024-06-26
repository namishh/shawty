#ifndef DATABASE
#define DATABASE

#include <sqlite3.h>

struct URL {
  char *shortened;
  char *target;
};

void seed_db();
void insert_url(char *target);
struct URL get_url(char *shortened);
void delete_url(char *shortened);
char *generate_html_list();
void delete_all_urls();

#endif // !DATABASE
