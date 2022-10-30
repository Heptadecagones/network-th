#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands.h"
#include "dbutil.h"
#include "server.h"

#include "client.h"

// If 1, reset DB at startup
#define RESET_DB 1

const char help_text[] = "List of commands :\n"
                         "\t/register [password]\n"
                         "\t/login [password]\n"
                         "\t/join [channel]\n"
                         "\t/leave\n"
                         "\t/whisper [user] [message]\n"
                         "\t/help\n"
                         "List of aliases :\n"
                         "\t/r for /register\n"
                         "\t/w for /whisper\n"
                         "\t/h for /help\n"
                         "The /leave command lets you go "
                         "back to the General channel\n";

// Needed function pointers before init
static void end(void);
static void clear_clients(Client *clients, int actual);

static void init(int reset) {
#ifdef WIN32
    WSADATA wsa;
    int err = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (err < 0) {
        puts("WSAStartup failed !");
        exit(EXIT_FAILURE);
    }
#endif
    // Call end if the server is terminated somehow (doesn't work for CTRL+\)
    atexit(end);
    // Initialize database
    printf("[DB INIT] ");
    init_db();
    if (reset) {
        printf("[DB RESET] ");
        reset_db();
    }
}

static void end(void) {
    printf("Closing server...\n");
#ifdef WIN32
    WSACleanup();
#endif
    // Close db connection
    close_db();
}

void write_prefix(Client c) {
    char *room_name = get_room_name_by_id(c.current_room_id);
    char room_prefix[35];
    snprintf(room_prefix, 35, "╔[%s]══════", room_name);
    write_client(c.sock, (char *)room_prefix);
    free(room_name);
}

Client *find_client_by_name(Client *clients, const char *name) {
    int i;
    for (i = 0; i < MAX_CLIENTS; i++)
        if (!strcmp(name, clients[i].name))
            return &clients[i];

    return NULL;
}

// TODO refactor this mess
static void app(void) {
    SOCKET sock = init_connection();
    char buffer[BUF_SIZE];
    /* the index for the array */
    int actual = 0;
    int max = sock;
    /* an array for all clients */
    Client clients[MAX_CLIENTS];

    fd_set rdfs;

    while (1) {
        int i = 0;
        FD_ZERO(&rdfs);

        /* add STDIN_FILENO */
        FD_SET(STDIN_FILENO, &rdfs);

        /* add the connection socket */
        FD_SET(sock, &rdfs);

        /* add socket of each client */
        for (i = 0; i < actual; i++) {
            FD_SET(clients[i].sock, &rdfs);
        }

        if (select(max + 1, &rdfs, NULL, NULL, NULL) == -1) {
            perror("select()");
            exit(errno);
        }

        /* something from standard input : i.e keyboard */
        if (FD_ISSET(STDIN_FILENO, &rdfs)) {
            /* stop process when type on keyboard */
            break;
        } else if (FD_ISSET(sock, &rdfs)) {
            /* new client */
            SOCKADDR_IN csin = {0};
            socklen_t sinsize = sizeof csin;
            int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
            if (csock == SOCKET_ERROR) {
                perror("accept()");
                continue;
            }

            /* after connecting the client sends its name */
            if (read_client(csock, buffer) == -1) {
                /* disconnected */
                continue;
            }

            printf("[LOG] %s connected.\n", buffer);

            /* what is the new maximum fd ? */
            max = csock > max ? csock : max;

            FD_SET(csock, &rdfs);

            Client c = {csock, "", 'g'};
            strncpy(c.name, buffer, BUF_SIZE - 1);

            c.id = -1; // Guest mode as long as the user doesn't log in
            c.current_room_id = 1;
            clients[actual] = c;
            actual++;

            // Send history to client
            int i, n_history_lines = 0;
            char **history = get_history_db(c.name, &n_history_lines);
            for (i = 0; i < n_history_lines; i++) {
                write_client(c.sock, history[i]);
                free(history[i]);
            }
            free(history);
            if (get_user_id(c.name) != -1)
                write_client(c.sock, "You still have to login.");
            write_client(c.sock, "Use /help to see available commands.");
            char *welcome_message = c.name;
            strcat(welcome_message, " connected.");
            send_message_to_room(clients, clients[actual - 1], actual,
                                 welcome_message, 1);
            write_prefix(c);
        } else {
            int i = 0;
            char *message, *timestamp;
            Client *target_client;
            int temp;
            for (i = 0; i < actual; i++) {
                /* a client is talking */
                if (FD_ISSET(clients[i].sock, &rdfs)) {
                    Client client = clients[i];
                    int c = read_client(clients[i].sock, buffer);
                    /* client disconnected */
                    if (c == 0) {
                        closesocket(clients[i].sock);
                        remove_client(clients, i, &actual);
                        strncpy(buffer, client.name, BUF_SIZE - 1);
                        strncat(buffer, " disconnected.",
                                BUF_SIZE - strlen(buffer) - 1);
                        send_message_to_room(clients, client, actual, buffer,
                                             1);
                    } else if (buffer[0] == '/') {
                        // Cas de l'envoi d'une commande
                        // read_command renvoi un pointeur déjà alloué en
                        // mémoire
                        char **res = read_command(buffer);
                        int room_id;
                        int whisper_message_size;
                        switch (atoi(res[0])) {
                        case 0: // Register
                            if (clients[i].id == -1) {
                                temp = register_user(clients[i].name, res[1]);
                                if (temp != -1) {
                                    write_client(clients[i].sock,
                                                 "Register successful, you may "
                                                 "now log in.");
                                } else
                                    write_client(
                                        clients[i].sock,
                                        "Error: Username might be taken.");
                            } else
                                write_client(clients[i].sock,
                                             "You are already logged in.");
                            break;
                        case 1: // Join a room
                            room_id = get_room_id_by_name(res[1]);
                            clients[i].current_room_id = room_id;
                            write_prefix(clients[i]);
                            break;
                        case 2:
                            clients[i].current_room_id = 1;
                            write_prefix(clients[i]);
                            break;
                        case 3:
                            // whisper
                            target_client =
                                find_client_by_name(clients, res[1]);
                            if (target_client != NULL) {
                                /* User found */
                                whisper_message_size =
                                    BUF_SIZE + strlen("-> whisper from ") +
                                    strlen(clients[i].name) + strlen(": ");
                                message = (char *)malloc(sizeof(char) *
                                                         whisper_message_size);
                                if (clients[i].id != -1 &&
                                    target_client->id != -1) {
                                    // Verify if SQlite version is good enough
                                    if (get_sqlite_minor_version() >= 53) {
                                        timestamp =
                                            save_message(res[2], clients[i].id,
                                                         target_client->id);
                                    } else {
                                        timestamp = "SQL version insufficient";
                                    }
                                }
                                snprintf(message, BUF_SIZE + 40,
                                         "[%s (UTC)] %s: %s\n", timestamp,
                                         clients[i].name, res[2]);
                                write_client(target_client->sock, message);
                                free(message);
                            } else {
                                /* User not found */
                                write_client(clients[i].sock,
                                             "User not found.");
                            }
                            break;
                        case 4:
                            write_client(clients[i].sock, help_text);
                            break;
                        case 5:
                            temp = auth_user(clients[i].name, res[1]);
                            if (clients[i].id != -1) {
                                write_client(clients[i].sock,
                                             "You are already logged in.");
                            } else if (temp != 0) {
                                write_client(clients[i].sock, "Authentified!");
                                clients[i].id = temp;
                            } else
                                write_client(clients[i].sock,
                                             "Invalid request, check your "
                                             "password.");

                            break;
                        case -2:
                            write_client(clients[i].sock,
                                         "Not enough arguments");
                            break;
                        default:
                            write_client(clients[i].sock,
                                         "Unknown command\r\n");
                        }
                        if (res[2] != NULL)
                            free(res[2]);
                        else
                            printf("res[2] is %s\r\n", res[2]);
                        if (res[1] != NULL)
                            free(res[1]);
                        else
                            printf("res[1] is %s\r\n", res[1]);
                        if (res[0] != NULL)
                            free(res[0]);
                        else
                            printf("res[0] is %s\r\n", res[0]);

                        free(res);
                    } else {
                        send_message_to_room(clients, client, actual, buffer,
                                             0);
                    }
                    break;
                }
            }
        }
    }

    clear_clients(clients, actual);
    end_connection(sock);
}

static void clear_clients(Client *clients, int actual) {
    int i = 0;
    for (i = 0; i < actual; i++) {
        closesocket(clients[i].sock);
    }
}

static void remove_client(Client *clients, int to_remove, int *actual) {
    /* we remove the client in the array */
    memmove(clients + to_remove, clients + to_remove + 1,
            (*actual - to_remove - 1) * sizeof(Client));
    /* number client - 1 */
    (*actual)--;
}

void send_message_to_room(Client *clients, Client sender, int actual,
                          const char *buffer, char from_server) {
    int i = 0;
    char message[BUF_SIZE];
    message[0] = 0;
    for (i = 0; i < actual; i++) {
        /* we don't send message to the sender */
        if (sender.sock != clients[i].sock) {
            if (from_server == 0) {
                strncpy(message, "║ ", BUF_SIZE - 1);
                strncat(message, sender.name,
                        sizeof message - strlen(message) - 1);
                strncat(message, ": ", sizeof message - strlen(message) - 1);
            } else
                strncpy(message, "❭❭ ", BUF_SIZE - 1);

            strncat(message, buffer, sizeof message - strlen(message) - 1);

            // Only broadcast to users in the same room
            if (clients[i].current_room_id == sender.current_room_id)
                write_client(clients[i].sock, message);
        }
    }
    strncat(message, CRLF, sizeof message - strlen(message) - 1);
}

static int init_connection(void) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    SOCKADDR_IN sin = {0};

    if (sock == INVALID_SOCKET) {
        perror("socket()");
        exit(errno);
    }

    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(PORT);
    sin.sin_family = AF_INET;

    if (bind(sock, (SOCKADDR *)&sin, sizeof sin) == SOCKET_ERROR) {
        perror("bind()");
        exit(errno);
    }

    if (listen(sock, MAX_CLIENTS) == SOCKET_ERROR) {
        perror("listen()");
        exit(errno);
    }

    return sock;
}

static void end_connection(int sock) { closesocket(sock); }

static int read_client(SOCKET sock, char *buffer) {
    int n = 0;

    if ((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0) {
        perror("recv()");
        /* if recv error we disonnect the client */
        n = 0;
    }

    buffer[n] = 0;

    return n;
}

static void write_client(SOCKET sock, const char *buffer) {
    if (send(sock, buffer, strlen(buffer), 0) < 0) {
        perror("send()");
        exit(errno);
    }
}

int main(int argc, char **argv) {
    init(RESET_DB);
    app();

    end();

    return EXIT_SUCCESS;
}
