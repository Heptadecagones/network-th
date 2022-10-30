#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "server.h"
#include "client.h"
#include "dbutil.h"
#include "commands.h"

// If 1, reset DB at startup
#define RESET_DB 1

// Needed function pointers before init
static void end(void);
static void clear_clients(Client *clients, int actual);

static void init(int reset)
{
#ifdef WIN32
    WSADATA wsa;
    int err = WSAStartup(MAKEWORD(2, 2), &wsa);
    if(err < 0)
    {
        puts("WSAStartup failed !");
        exit(EXIT_FAILURE);
    }
#endif
    // Call end if the server is terminated somehow (doesn't work for CTRL+\)
    atexit(end);
    // Initialize database
    printf("[DB INIT] "); 
    init_db();
    if(reset)
    {
        printf("[DB RESET] "); 
        reset_db();
    }
}

static void end(void)
{
    printf("Closing server...\n");
#ifdef WIN32
    WSACleanup();
#endif
    // Close db connection
    close_db();
}

void write_prefix(Client c) {
    printf("Entered write_prefix\n");
    char *room_name = get_room_name_by_id(c.current_room_id);
    char room_prefix[35];
    snprintf(room_prefix, 35, "╔[%s]═══\n", room_name);
    write_client(c.sock, (char *)room_prefix);
}

SOCKET find_client_socket_by_name(Client* clients, const char* name)
{
    int i;
    int sock = -1;  // Default value if the user is not found
    for(i = 0; i < MAX_CLIENTS; i++ )
    {
        if(!strcmp(name, clients[i].name))
        {
            sock = clients[i].sock;
            break;
        }
    }

    return sock;
}

//TODO refactor this mess
static void app(void)
{
    SOCKET sock = init_connection();
    char buffer[BUF_SIZE];
    /* the index for the array */
    int actual = 0;
    int max = sock;
    /* an array for all clients */
    Client clients[MAX_CLIENTS];

    fd_set rdfs;

    while(1)
    {
        int i = 0;
        FD_ZERO(&rdfs);

        /* add STDIN_FILENO */
        FD_SET(STDIN_FILENO, &rdfs);

        /* add the connection socket */
        FD_SET(sock, &rdfs);

        /* add socket of each client */
        for(i = 0; i < actual; i++)
        {
            FD_SET(clients[i].sock, &rdfs);
        }

        if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
        {
            perror("select()");
            exit(errno);
        }

        /* something from standard input : i.e keyboard */
        if(FD_ISSET(STDIN_FILENO, &rdfs))
        {
            /* stop process when type on keyboard */
            break;
        }
        else if(FD_ISSET(sock, &rdfs))
        {
            /* new client */
            SOCKADDR_IN csin = { 0 };
            socklen_t sinsize = sizeof csin;
            int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
            if(csock == SOCKET_ERROR)
            {
                perror("accept()");
                continue;
            }

            /* after connecting the client sends its name */
            if(read_client(csock, buffer) == -1)
            {
                /* disconnected */
                continue;
            }

            printf("[LOG] %s connected.\n", buffer);

            /* what is the new maximum fd ? */
            max = csock > max ? csock : max;

            FD_SET(csock, &rdfs);

            Client c = { csock, "", 'g'};
            strncpy(c.name, buffer, BUF_SIZE - 1);
            clients[actual] = c;
            actual++;

            c.id = get_user_id(c.name);
            c.current_room_id = 1;

            // Send history to client
            int i, n_history_lines = 0;
            char **history = get_history_db(c.name, &n_history_lines);
            for(i = 0; i < n_history_lines; i++)
            {
                write_client(c.sock, history[i]);
                free(history[i]);
            }
            free(history);
            write_prefix(c);
        }
        else
        {
            int i = 0;
            char* server_to_client_message;
            char* whisper_message;
            SOCKET whisper_target_socket;
            for(i = 0; i < actual; i++)
            {
                /* a client is talking */
                if(FD_ISSET(clients[i].sock, &rdfs))
                {
                    Client client = clients[i];
                    int c = read_client(clients[i].sock, buffer);
                    /* client disconnected */
                    if(c == 0)
                    {
                        closesocket(clients[i].sock);
                        remove_client(clients, i, &actual);
                        strncpy(buffer, client.name, BUF_SIZE - 1);
                        strncat(buffer, " disconnected.", BUF_SIZE - strlen(buffer) - 1);
                        send_message_to_room(clients, client, actual, buffer, 1);
                    }
                    else if(buffer[0] == '/')
                    {
                        // Cas de l'envoi d'une commande
                        // read_command renvoi un pointeur déjà alloué en mémoire
                        char **res = read_command(buffer);
                        int room_id;
                        int whisper_message_size;
                        printf("res[0] is %d \n", atoi(res[0]));
                        switch(atoi(res[0]))
                        {
                            case 0:
                                auth_user(atoi(res[1]), res[2]);
                                break;
                            case 1:
                                printf("Entered switch join\n");
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
                                whisper_target_socket = find_client_socket_by_name(clients, res[1]);
                                if(whisper_target_socket != -1)
                                {
                                    /* User found */
                                    whisper_message_size = BUF_SIZE + strlen("-> whisper from ") + strlen(clients[i].name) + strlen(": ");
                                    whisper_message = (char*) malloc(sizeof(char)*whisper_message_size);
                                    strcpy(whisper_message, "-> whisper from ");
                                    strcat(whisper_message, clients[i].name);
                                    strcat(whisper_message, ": ");
                                    strcat(whisper_message, res[2]);
                                    write_client(whisper_target_socket, whisper_message);
                                    free(whisper_message);
                                }
                                else
                                {
                                    /* User not found */
                                    write_client(clients[i].sock, "User not found");
                                }
                                break;
                            case 4:
                                server_to_client_message = (char*) malloc(sizeof(char)*BUF_SIZE);
                                strcat(server_to_client_message, "List of commands :\n");
                                strcat(server_to_client_message, "\t/register [id] [password]\n");
                                strcat(server_to_client_message, "\t/join [channel]\n");
                                strcat(server_to_client_message, "\t/leave\n");
                                strcat(server_to_client_message, "\t/whisper [user] [message]\n");
                                strcat(server_to_client_message, "\t/help\n");
                                strcat(server_to_client_message, "List of aliases :\n");
                                strcat(server_to_client_message, "\t/r for /register\n");
                                strcat(server_to_client_message, "\t/w for /whisper\n");
                                strcat(server_to_client_message, "\t/h for /help\n");
                                strcat(server_to_client_message, "The /leave command lets you go back to the General channel\n");

                                write_client(clients[i].sock, server_to_client_message);
                                free(server_to_client_message);
                                break;
                            case -2:
                                write_client(clients[i].sock, "Not enough arguments");
                                break;
                            default:
                                write_client(clients[i].sock, "Unknown command\r\n");
                        }
                        free(res[2]);
                        free(res[1]);
                        free(res[0]);
                        free(res);
                    }
                    else
                    {
                        send_message_to_room(clients, client, actual, buffer, 0);
                    }
                    break;
                }
            }
        }
    }

    clear_clients(clients, actual);
    end_connection(sock);
}

static void clear_clients(Client *clients, int actual)
{
    int i = 0;
    for(i = 0; i < actual; i++)
    {
        closesocket(clients[i].sock);
    }
}

static void remove_client(Client *clients, int to_remove, int *actual)
{
    /* we remove the client in the array */
    memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
    /* number client - 1 */
    (*actual)--;
}

static char* send_message_to_room(Client *clients, Client sender, int actual, const char *buffer, char from_server)
{
    int i = 0;
    char message[BUF_SIZE];
    char *res;
    message[0] = 0;
    for(i = 0; i < actual; i++)
    {
        /* we don't send message to the sender */
        if(sender.sock != clients[i].sock)
        {
            if(from_server == 0) {
                strncpy(message, sender.name, BUF_SIZE - 1);
                strncat(message, ": ", sizeof message - strlen(message) - 1);
            }
            else
                strncpy(message, "❭❭ ", BUF_SIZE - 1);

            strncat(message, buffer, sizeof message - strlen(message) - 1);

            // Only broadcast to users in the same room
            if(clients[i].current_room_id == sender.current_room_id)
                write_client(clients[i].sock, message);
        }
    }
    strncat(message, CRLF, sizeof message - strlen(message) - 1);
    res = (char*) malloc(sizeof(char)*strlen(message));
    strncpy(res, message, strlen(message)-1);
    return res;
}

static int init_connection(void)
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    SOCKADDR_IN sin = { 0 };

    if(sock == INVALID_SOCKET)
    {
        perror("socket()");
        exit(errno);
    }

    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(PORT);
    sin.sin_family = AF_INET;

    if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
    {
        perror("bind()");
        exit(errno);
    }

    if(listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
    {
        perror("listen()");
        exit(errno);
    }

    return sock;
}

static void end_connection(int sock)
{
    closesocket(sock);
}

static int read_client(SOCKET sock, char *buffer)
{
    int n = 0;

    if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
    {
        perror("recv()");
        /* if recv error we disonnect the client */
        n = 0;
    }

    buffer[n] = 0;

    return n;
}

static void write_client(SOCKET sock, const char *buffer)
{
    if(send(sock, buffer, strlen(buffer), 0) < 0)
    {
        perror("send()");
        exit(errno);
    }
}

int main(int argc, char **argv)
{
    init(RESET_DB);
    app();

    end();

    return EXIT_SUCCESS;
}
