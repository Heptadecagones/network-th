#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "commands.h"

#define BUF_SIZE    1024

static int read_command(const char *command)
{
    char *arg1[BUF_SIZE];
    char *arg1[BUF_SIZE];
    const char delimiter = ' '; 
    int spaceIndex;

    char *temp = strtok(command, delimiter);

    // A vérifier : command est interprêté sans prendre en compte l'espacement du \ 
    if (!strncmp("\\join", command, 5)))       // Checks only the first 5 characters of command (so it doesn't take the argument)
    {
        temp = strtok(toDo, delimiter);
        strcpy(arg1, temp);                             // On suppose que le message envoyé est bien \join et pas \\join.
        // Commande de join
    }
    else if(!strncmp("\\leave", command, 6))
    {
        temp = strtok(toDo, delimiter);
        strcpy(arg1, temp);
        // Commande de leave
    }
    else if(!strncmp("\\whisper", commande, 8) || !strncmp("\\w", toDo, 2))
    {
        temp = strtok(toDo, delimiter);
        strcpy(arg1, temp);
        temp = strtok(temp, delimiter);
        strcpy(arg2, temp);
        // Commande de whisper
    }
    
    return 0;
}