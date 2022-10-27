#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "commands.h"

#define BUF_SIZE    1024

static int read_command(const char *command)
{
    char * restrict arg1 = (char*) malloc(sizeof(char)*BUF_SIZE);
    char * restrict arg2 = (char*) malloc(sizeof(char)*BUF_SIZE);
    const char* restrict delimiter = " "; 

    char commandCopy[strlen(command) + 1];
    strcpy(commandCopy, command);

    char *temp = strtok(commandCopy, delimiter);

    /* A vérifier : command est interprêté sans prendre en compte l'espacement du \ , */
    if (!strncmp("\\join", command, 5))       // Checks only the first 5 characters of command (so it doesn't take the argument)
    {
        strcpy(arg1, temp);                             // On suppose que le message envoyé est bien \join et pas \\join.
        // Commande de join
        printf("commande \\join reçue avec comme paramètre : %s\r\n", arg1);
    }
    else if(!strncmp("\\leave", command, 6))
    {
        strcpy(arg1, temp);
        // Commande de leave
        printf("commande \\leave reçue avec comme paramètre : %s\r\n", arg1);
    }
    else if(!strncmp("\\whisper", command, 8) || !strncmp("\\w", command, 2))
    {
        strcpy(arg1, temp);
        temp = strtok(temp, delimiter);
        strcpy(arg2, temp);
        // Commande de whisper
        printf("commande \\whisper reçue avec comme paramètre : %s et %s\r\n", arg1, arg2);
    }
    else {
        printf("Commande non reconnue\r\n");
    }
    free(arg1);
    free(arg2);
    free(temp);
    
    return 0;
}