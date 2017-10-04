// for strchrnul
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
 * tokenizes a string for command parsing
 *
 * commands: command to be parsed
 *
 * ret: array of char pointers (strings) of tokenized values
 */

char** tokenize(char* commands) {
    // TODO: remove magic number
    char** toRet = (char**)calloc(1024, sizeof(char*));
    // TODO: handle errors from calloc

    char* word;
    char* prev_word = commands;
    int comm_i = 0;
    int last_word = 0;
    // tokenize on spaces

    while(1) {
        word = strchrnul(prev_word, ' ');
        if (*word == '\0') {
            last_word = 1;
        }

        // + 1 for null termination
        int word_size = word - prev_word + 1;
        // TODO handle malloc return
        toRet[comm_i] = (char*) malloc(word_size * sizeof(char));
        // TODO handle return
        strncpy(toRet[comm_i], prev_word, word_size);
        toRet[comm_i][word_size-1] = '\0';

        // skip the token
        prev_word = word + 1;
        comm_i++;

        if (last_word) 
            break;
    }


    return toRet;
}


// for testing purposes
// TODO remove
int main() {
    char* command = "echo    hi";
    char** comms = tokenize(command);

    char* comm;
    int i = 0;
    while(1) {
        comm = comms[i];
        if (comm == NULL) {
            break;
        }
        printf("%s\n", comm);
        i++;

    }


    return 0;
}