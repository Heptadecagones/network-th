#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands.h"

#define BUF_SIZE 1024
#define MAX_ARG 2

/* @author: htricot
 * Parseur de commandes
 */

// #define COMMAND_AMOUNT  4

/* res[0] is the command to execute, res[1] is the first argument and res[2] is
   the second (if exist). Possible values for res[0] are:
        - 0 : /register (res[1] and res[2] are set)
        - 1 : /join (res[1] is set)
        - 2 : /leave
        - 3 : /whipser (res[1] and res[2] are set)
        - 4 : /help
        - -1 : command not found
        - -2 : arguments missing
*/
char **read_command(const char *command) {
    char **res = (char **)malloc(sizeof(char *) * (MAX_ARG + 1));
    res[0] = (char *)malloc(sizeof(char) * 10);
    for (int i = 0; i < MAX_ARG; i++) {
        res[i + 1] = (char *restrict)malloc(sizeof(char) * BUF_SIZE);
        strcpy(res[i + 1], "\0");
    }
    const char *restrict delimiter = " ";
    int extraction_result;
    int arg_amount;

    /*char commandCopy[strlen(command) + 1];
    strcpy(commandCopy, command);
    char *temp = strtok(commandCopy, delimiter);*/
    if (!strncmp("/register", command, 9) || !strncmp("/r", command, 2)) {
        /* /register [password] */
        arg_amount = 1;
        strcpy(res[0], "0");

        extraction_result =
            extract_arguments(res, arg_amount, command, delimiter, 0);
        /* Case where there are not enough arguments */
        if (extraction_result)
            strcpy(res[0], "-2");

        printf("[CMD PARSER] /register %s\r\n", res[1]);

    } else if (!strncmp("/join", command,
                        5)) // Checks only the first 5 characters of command (so
                            // it doesn't take the argument)
    {
        /* /join [chanel] */
        arg_amount = 1;
        strcpy(res[0], "1");

        extraction_result =
            extract_arguments(res, arg_amount, command, delimiter, 0);
        /* Case where there are not enough arguments */
        if (extraction_result)
            strcpy(res[0], "-2");
        printf("[CMD PARSER] /join %s\r\n", res[1]);
    } else if (!strncmp("/leave", command, 6)) {
        /* /leave */
        strcpy(res[0], "2");

        printf("[CMD PARSER] /leave");
    } else if (!strncmp("/whisper", command, 8) || !strncmp("/w", command, 2)) {
        /* /whisper [user] [message]*/
        arg_amount = 2;
        strcpy(res[0], "3");

        extraction_result =
            extract_arguments(res, arg_amount, command, delimiter, 1);
        /* Case where there are not enough arguments */
        if (extraction_result)
            strcpy(res[0], "-2");
        // Commande de whisper
        printf("[CMD PARSER] /whisper %s %s\r\n", res[1], res[2]);
    } else if (!strncmp("/help", command, 5) || !strncmp("/h", command, 2)) {
        /* /help */
        strcpy(res[0], "4");
    } else if (!strncmp("/login", command, 6) || !strncmp("/r", command, 2)) {
        /* /login [password] */
        arg_amount = 1;
        strcpy(res[0], "5");

        extraction_result =
            extract_arguments(res, arg_amount, command, delimiter, 0);
        /* Case where there are not enough arguments */
        if (extraction_result)
            strcpy(res[0], "-2");

        printf("[CMD PARSER] /login %s\r\n", res[1]);
    } else if (!strncmp("/channels", command, 6) || !strncmp("/r", command, 2)) {
        /* /login [password] */
        arg_amount = 0;
        strcpy(res[0], "6");

        extraction_result =
            extract_arguments(res, arg_amount, command, delimiter, 0);
        /* Case where there are not enough arguments */
        if (extraction_result)
            strcpy(res[0], "-2");

        printf("[CMD PARSER] /login %s\r\n", res[1]);
    } else {
        printf("[CMD PARSER] Commande non reconnue\r\n");
        strcpy(res[0], "-1");
    }

    return res;
}

/* Returns -1 if the number of arg_amount is more than the value of MAX_ARG.
   Otherwise sends the number of the first missing argument.
   Set last_arg_is_message to 1 if the last argument must consider all
   last words as one argument. */
int extract_arguments(char **arg_dest, int arg_amount, const char *command,
                      const char *restrict delimiter, int last_arg_is_message) {
    int res = 0;
    int i;

    char commandCopy[strlen(command) + 1];
    strcpy(commandCopy, command);
    char *temp = strtok(commandCopy, delimiter);
    if (temp != NULL)
        temp =
            strtok(NULL, delimiter); // Does not extract the command afterwards

    if (arg_amount > MAX_ARG) {
        perror("extract_arguments: arg_amount > MAX_ARG.\r\n");
        return -1;
    }
    for (i = 1; i <= arg_amount; i++) {
        if (temp == NULL) {
            res = i;
        } else {

            if (last_arg_is_message && i == arg_amount) {
                do {
                    strcat(arg_dest[i], temp);
                    strcat(arg_dest[i], " \0");
                    temp = strtok(NULL, delimiter);
                } while (temp != NULL);
                /* This is considering only the last argument may be a phrase.
                 */
            } else {
                strcpy(arg_dest[i], temp);
                temp = strtok(NULL, delimiter);
            }
        }
    }

    return res;
}
