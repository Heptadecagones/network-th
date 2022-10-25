#include <sqlite3.h>

sqlite3_stmt* query_db(char*);
sqlite3_stmt* auth_user(char* password);

int init_db(void);
int close_db(void);
int reset_db(void);
