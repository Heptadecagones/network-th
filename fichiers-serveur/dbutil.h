#include <sqlite3.h>

sqlite3_stmt* query_db(char*);
int auth_user(int user_id, char* password);
int subscribe_user_to_room(int sock, char *client_name, char *room_name);
char** get_history_db(char* client_name, int* n_history_lines);
int get_user_id(char* client_name);
char* get_room_name_by_id(int room_id);

int init_db(void);
int close_db(void);
int reset_db(void);
