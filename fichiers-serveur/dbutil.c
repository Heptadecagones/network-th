#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dbutil.h"

/* @omi
 * Module d'interface SQLite
 * On va faire un truc style objets métier puisque ça a l'air de bien marcher
 */

#ifndef BUF_SIZE
#define BUF_SIZE 1024
#endif

#ifndef HISTORY_SIZE
#define HISTORY_SIZE 2048
#endif

static sqlite3 *db = NULL;

// DB operation error checking method
void check_error(int result_code) {
    if (result_code != SQLITE_OK) {
        printf("[DB CHK] %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
    }
}

// General purpose DB interface function
// Can be used to bypass premade methods
sqlite3_stmt *query_db(char *sql) {
    // Query result
    sqlite3_stmt *result;
    // Return code
    int rc = sqlite3_prepare_v2(db, sql, -1, &result, 0);
    if (rc == SQLITE_OK)
        sqlite3_bind_int(result, 1, 3);
    else
        fprintf(stderr, "[DB QUERY] Failed to execute statement: %s\n",
                sqlite3_errmsg(db));
    return result;
}

// Check password input against stored password
int auth_user(int user_id, char *password) {
    sqlite3_stmt *res;
    int rc;
    char *sql =
        "SELECT UserID FROM Users WHERE UserID == :id AND Password == :pw";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    check_error(rc);

    // Bind value to query
    rc = sqlite3_bind_int(res, 1, user_id); 
    rc = sqlite3_bind_text(res, 1, password, -1, NULL);
    check_error(rc);

    // Get result and end transaction
    rc = sqlite3_step(res);
    sqlite3_finalize(res);

    // Return 0 if done, 1 otherwise
    return rc == SQLITE_DONE ? 0 : 1;
}

int subscribe_user_to_room(int sock, char *client_name) {
    // Pre-built statement
    sqlite3_stmt *res;
    // Result code
    int rc;

    // Prepare parametrized query
    char *sql = "INSERT INTO User_rooms (User, room) VALUES (:user, :room)";
    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    check_error(rc);

    // Bind value to query
    rc = sqlite3_bind_text(res, 1, client_name, -1, NULL);
    check_error(rc);

    // Get result and end transaction
    rc = sqlite3_step(res);
    sqlite3_finalize(res);

    // Return 0 if done, 1 otherwise
    return rc == SQLITE_DONE ? 0 : 1;
}

int get_user_id(char *client_name) {
    // Pre-built statement
    sqlite3_stmt *res;
    // Result code
    int rc;

    // Prepare parametrized query
    char *sql = "SELECT UserID from Users WHERE Users.Username = :name;";
    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    check_error(rc);

    // Bind value to query
    rc = sqlite3_bind_text(res, 1, client_name, -1, NULL);
    check_error(rc);

    // Get result and end transaction
    rc = sqlite3_step(res);
    check_error(rc);
    int user_id = sqlite3_column_int(res, 0);
    sqlite3_finalize(res);

    // Return user_id if done, -1 otherwise (negative ID = guest)
    return rc == SQLITE_DONE ? -1 : user_id;
}

// Send messages in the order they arrived
char **get_history_db(char *client_name, int *n_lines) {
    // Return pointer
    char **history = (char **)malloc(sizeof(char *) * HISTORY_SIZE);

    // Pre-built statement
    sqlite3_stmt *res;
    // Result code
    int rc;

    // Prepare parametrized query
    char *sql = ""
                "SELECT Messages.Timestamp, Users.Username, Messages.Contents "
                "FROM Messages "
                "INNER JOIN Users ON Messages.Recipient = Users.UserID "
                "WHERE Users.Username = :name "
                "ORDER BY Messages.Timestamp;";
    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    check_error(rc);

    // Bind value to query
    rc = sqlite3_bind_text(res, 1, client_name, -1, NULL);
    check_error(rc);

    rc = sqlite3_step(res);

    *n_lines = 0;
    while (rc == SQLITE_ROW) {
        // Evaluate query
        if (rc == SQLITE_ROW) {
            // We need to print a message with max size BUF_SIZE + timestamp and
            // name, therefore 40 additional characters are reserved
            char message[BUF_SIZE + 40];
            snprintf(message, BUF_SIZE + 40, "[%s (UTC)] %s: %s\n",
                     sqlite3_column_text(res, 0), sqlite3_column_text(res, 1),
                     sqlite3_column_text(res, 2));
            history[*n_lines] = (char *)malloc(sizeof(char) * (BUF_SIZE + 40));
            strcpy(history[*n_lines], message);
            *n_lines += 1;
        } else {
            check_error(rc);
        }

        rc = sqlite3_step(res);
    }

    sqlite3_finalize(res);
    return history;
}

// Clean slate!
int reset_db() {
    char *err_msg = 0;
    char *sql = ""
                // Users table
                "DROP TABLE IF EXISTS Users;"
                "CREATE TABLE Users("
                "UserID INTEGER PRIMARY KEY AUTOINCREMENT,"
                "Username TEXT,"
                "Password TEXT);"
                // Messages table
                "DROP TABLE IF EXISTS Messages;"
                "CREATE TABLE Messages("
                "MessageID INTEGER PRIMARY KEY AUTOINCREMENT,"
                "Timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                "Author INT,"
                "Recipient INT,"
                "Contents TEXT,"
                "FOREIGN KEY (Author) REFERENCES Users(UserID),"
                "FOREIGN KEY (Recipient) REFERENCES Users(UserID));"
                // Rooms table
                "DROP TABLE IF EXISTS Rooms;"
                "CREATE TABLE Rooms("
                "RoomID INTEGER PRIMARY KEY AUTOINCREMENT,"
                "Name TEXT);"
                // User_Rooms table
                "DROP TABLE IF EXISTS User_rooms;"
                "CREATE TABLE User_rooms("
                "User INT,"
                "Room INT,"
                "UNIQUE(User, Room),"
                "FOREIGN KEY (User) REFERENCES Users(UserID),"
                "FOREIGN KEY (Room) REFERENCES Rooms(RoomID));"
                // Dummy values
                "INSERT INTO Users (Username, Password)"
                "VALUES ('John', 'Doe'), ('Duke', 'Nukem');"
                "INSERT INTO Messages (Author, Recipient, Contents)"
                "VALUES (1, 2, 'Hi Duke, it is Joe!'), (2, 1, 'Hello Joe, it "
                "is I, Duke.');"
                "INSERT INTO Rooms (Name)"
                "VALUES ('Botanique'), ('Jardinage'), ('Horticulture');";
    // Return code
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != 0)
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

// Avoids including sqlite3 in server.c (maintains loose coupling)
int close_db() { return sqlite3_close(db); }
