#ifndef CLIENT_H
#define CLIENT_H

#include "server.h"

typedef struct
{
    SOCKET sock;
    char name[BUF_SIZE];
    int id;
}Client;

#endif /* guard */
