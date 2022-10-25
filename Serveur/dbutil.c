#include <sqlite3.h>
#include <stdio.h>

#include "dbutil.h"

/* @omi
 * Module d'interface SQLite
 * On va faire un truc style objets métier puisque ça a l'air de bien marcher
 */


static sqlite3 *db = NULL;

// General purpose DB interface function
// Can be used to bypass premade methods
int query_db(char* sql) {
    char *err_msg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);  
        return 1;
    }
    return 0;
}

// 0 = OK, 1 = NOK
int init_db() {

    int rc = sqlite3_open("server_data.db", &db);

    if (rc != SQLITE_OK) {

        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);  
        return 1;
    }

    return 0;
}

// Avoids including sqlite3 in server.c
int close_db() {
    return sqlite3_close(db);
}
