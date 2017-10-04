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

// TODO: handle errors everywhere
char** tokenize(char* commands) {
    // TODO: remove magic number
    char** toRet = (char**)calloc(1024, sizeof(char*));

    int i = 0;
    int word_start = 0;
    int comm_num = 0;
    int word_size;

    while(1) {
        char ch = commands[i];
        if (ch == ' ') {
            // + 1 for null termination
            word_size = i - word_start + 1;

            toRet[comm_num] = (char*) malloc(word_size * sizeof(char));
            strncpy(toRet[comm_num], commands + word_start, word_size);
            toRet[comm_num][word_size-1] = '\0';
            
            comm_num++;
            
            // skip repeated spaces
            while (commands[i] == ' ') {
                i++;
            }
            word_start = i;
        } else if (ch == '\0') {
            // + 1 for null termination
            word_size = i - word_start + 1;

            toRet[comm_num] = (char*) malloc(word_size * sizeof(char));
            strncpy(toRet[comm_num], commands + word_start, word_size);
            toRet[comm_num][word_size-1] = '\0';
            
            break;
        } else {
            i++;
        }

    }


    return toRet;
}


// for testing purposes
// TODO remove
int main() {
    char* command = "echo     hi";
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