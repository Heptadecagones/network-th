#ifndef CLIENT_H
#define CLIENT_H

#include "server.h"

typedef struct
{
    SOCKET sock;
    char name[BUF_SIZE];
    char mode; // g = guest, u = logged user, a = admin
    int id;
}Client;

#endif /* guard */
