#include <sqlite3.h>

sqlite3_stmt *query_db(char *);
int register_user(char *uname, char *password);
int auth_user(char *username, char *password);
int subscribe_user_to_room(int sock, char *client_name, char *room_name);
char **get_history_db(char *client_name, int *n_history_lines);
int get_user_id(char *client_name);
char *save_message(char *msg, int sender_id, int dest_id);
char *get_room_name_by_id(int room_id);
int get_room_id_by_name(char *room_name);
char get_sqlite_minor_version();
int init_db(void);
int close_db(void);
int reset_db(void);
