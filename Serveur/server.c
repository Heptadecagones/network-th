#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "server.h"
#include "client.h"
#include "dbutil.h"

static void end(void);

static void init(void)
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
}

static void end(void)
{
    printf("Goodbye.\n");
#ifdef WIN32
    WSACleanup();
#endif
    // Close db connection
    close_db();
}

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

    char history[BUF_SIZE*HISTORY_SIZE];   // On considère pour l'instant qu'on peut garder HISTORY_SIZE messages

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

            /* what is the new maximum fd ? */
            max = csock > max ? csock : max;

            FD_SET(csock, &rdfs);

            Client c = { csock };
            strncpy(c.name, buffer, BUF_SIZE - 1);
            clients[actual] = c;
            actual++;

            send_history(csock, history);
        }
        else
        {
            int i = 0;
            char* message_send;
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
                        strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                        printf("before send message (client disconnected)\r\n");
                        message_send = send_message_to_all_clients(clients, client, actual, buffer, 1);
                    }
                    else
                    {
                        printf("before send message\r\n");
                        message_send = send_message_to_all_clients(clients, client, actual, buffer, 0);
                    }
                    // Gestion de l'historique
                    printf("before add to history: message_send is : %s\r\n", message_send);
                    strncat(history, message_send, (BUF_SIZE*HISTORY_SIZE) - strlen(message_send) - 1);
                    free(message_send);     // Car on a alloué avec malloc message_send dans send_message_to_all_clients
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

static char* send_message_to_all_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server)
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
            if(from_server == 0)
            {
                strncpy(message, sender.name, BUF_SIZE - 1);
                strncat(message, " : ", sizeof message - strlen(message) - 1);
            }
            strncat(message, buffer, sizeof message - strlen(message) - 1);
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

/* Envoi l'ensemble des messages envoyés depuis le début */
static void send_history(SOCKET sock, const char *history)
{
    /* On considère actuellement que l'envoi de l'historique est l'équivalent de l'envoi d'un message, la méthode peut être appelée à changer, d'où une méthode spécifique  */
    write_client(sock, history);
}

int main(int argc, char **argv)
{
    init();
    app();

    end();

    return EXIT_SUCCESS;
}
