#include <sqlite3.h>

sqlite3_stmt* query_db(char*);
sqlite3_stmt* auth_user(int user_id, char* password);
char** get_history_db(char* client_name, int* n_history_lines);
int get_user_id(char* client_name);


int init_db(void);
int close_db(void);
int reset_db(void);
