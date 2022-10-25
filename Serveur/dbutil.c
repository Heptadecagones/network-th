#include <sqlite3.h>
#include <stdio.h>

#include "dbutil.h"

/* @omi
 * Module d'interface SQLite
 * On va faire un truc style objets métier puisque ça a l'air de bien marcher
 */

static sqlite3 *db = NULL;

// Registers a message in the database
int log_message() {
    return 0;
}

// General purpose DB interface function
// Can be used to bypass premade methods
sqlite3_stmt* query_db(char* sql) {
    sqlite3_stmt *res;
    int rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    if (rc == SQLITE_OK)
        sqlite3_bind_int(res, 1, 3);
    else
        fprintf(stderr, "[DB QUERY] Failed to execute statement: %s\n", sqlite3_errmsg(db));
    return res;
}

// Check password input against stored password
sqlite3_stmt* auth_user(char* password) {
    char* sql = "TODO";
    return query_db(sql);
}

// Clean slate!
int reset_db() {
    char *err_msg = 0;
    char *sql = ""
        "DROP TABLE IF EXISTS Users;" 
        "CREATE TABLE Users("
        "UserID INT,"
        "Name TEXT,"
        "Password TEXT,"
        "PRIMARY KEY (UserId));"
        "DROP TABLE IF EXISTS Messages;" 
        "CREATE TABLE Messages("
        "MessageID INT,"
        "Author INT,"
        "Contents TEXT,"
        "PRIMARY KEY (MessageID),"
        "FOREIGN KEY (Author) REFERENCES Users(UserID));";
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if(err_msg != 0)
        printf("%s\n", err_msg);
    else
        printf("OK\n");
    
    return rc;
}

// 0 = OK, 1 = NOK
int init_db() {
    int rc = sqlite3_open("server_data.db", &db);

    if (rc != SQLITE_OK) {
        printf("%s\n", sqlite3_errmsg(db));
        sqlite3_close(db);  
        return 1;
    }
    printf("OK\n");
    return rc;
}

// Avoids including sqlite3 in server.c
int close_db() {
    return sqlite3_close(db);
}
