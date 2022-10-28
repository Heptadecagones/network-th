#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "commands.h"

#define BUF_SIZE    1024
#define MAX_ARG     2
//#define COMMAND_AMOUNT  4

char **read_command(const char *command)
{
    printf("Allocating res\r\n");;
    char **res = (char**) malloc(sizeof(char*) * (MAX_ARG + 1));
    res[0] = (char*) malloc(sizeof(char)*10);
    for(int i = 0; i < MAX_ARG; i++)
    {
        printf("Allocating argument number %d\r\n", i+1);
        res[i+1] = (char * restrict) malloc(sizeof(char)*BUF_SIZE);
        res[i+1] = "\0";
        printf("Argument number %d\r\n", i+1);
    }
    printf("Allocating complete\r\n");
    const char* restrict delimiter = " ";

    char commandCopy[strlen(command) + 1];
    strcat(commandCopy, command);
    printf("commandCopy dealt with\r\n");
    char *temp = strtok(commandCopy, delimiter);
    printf("obtained first temp : %s\r\n", temp);
    if(!strncmp("/register", command, 8) || !strncmp("/r", command, 2))
    {
        /* /register [id] [password] */
        strcat(res[0], "0");

        temp = strtok(NULL, delimiter);
        strcpy(res[1], temp);
        temp = strtok(NULL, delimiter);
        strcpy(res[2], temp);
        
        printf("commande /register reçue avec comme paramètre : %s et %s\r\n", res[1], res[2]);
    }
    else if (!strncmp("/join", command, 5))       // Checks only the first 5 characters of command (so it doesn't take the argument)
    {
        /* /join [chanel] */
        strcat(res[0], "1");
        printf("Dealt with command\r\n");
        temp = strtok(NULL, delimiter);
        printf("before dealing with arg1\r\n");
        if(temp == NULL) printf("arg1 is null");
        strcpy(res[1], temp);                             // On suppose que le message envoyé est bien /join
        printf("commande /join reçue avec comme paramètre : %s\r\n", res[1]);
    }
    else if(!strncmp("/leave", command, 6))
    {
        strcat(res[0], "2");

        temp = strtok(NULL, delimiter);
        strcpy(res[1], temp);
        // Commande de leave
        printf("commande /leave reçue avec comme paramètre : %s\r\n", res[1]);
    }
    else if(!strncmp("/whisper", command, 8) || !strncmp("/w", command, 2))
    {
        strcat(res[0], "3");

        temp = strtok(NULL, delimiter);
        strcpy(res[1], temp);
        temp = strtok(NULL, delimiter);
        strcpy(res[2], temp);
        // Commande de whisper
        printf("commande /whisper reçue avec comme paramètre : %s et %s\r\n", res[1], res[2]);
    }
    else {
        printf("Commande non reconnue\r\n");
    }
    
    return res;
}
