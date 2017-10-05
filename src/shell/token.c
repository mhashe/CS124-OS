#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "token.h"

/*
 * copy src[word_start:word_start+word_size] into dest[i] as a null term string
 */ 
void copy_comm(char** dest, char* src, int word_start, int word_size, int i) {
    dest[i] = (char*) malloc(word_size * sizeof(char));
    strncpy(dest[i], src + word_start, word_size);
    dest[i][word_size-1] = '\0';
}

/*
 * checks if a string[start_i:end_i] is a positive number
 *
 * return:
 *     1 if valid positive number
 *     0 otherwise
 * 
 */
int is_number(char* str, int start_i, int end_i) {
    // do not allow for number of length 0
    if (end_i == start_i) {
        return 0;
    }

    for (int i = start_i; i < end_i; i++) {
        if (!isdigit(str[i])) {
            return 0;
        }
    }
    return 1;
}

/*
 * tokenizes a string for command parsing
 *
 * commands: command to be parsed
 *
 * ret: array of char pointers (strings) of tokenized values
 */
// TODO handle errors everywhere
// TODO remove repeated code
// TODO handle integer file descriptor
char** tokenize(char* commands) {
    // TODO: remove magic number
    char** toRet = (char**)calloc(1024, sizeof(char*));

    int i = 0;
    int word_start = 0;
    int comm_num = 0;
    int word_size;

    while(1) {
        char ch = commands[i];
        if (ch =='\"') {
            // treat everything until closing quote as a string

            i = (int)(strchr(commands + i + 1, '\"') - commands);

            // TODO handle when quotes happen in middle eg hel"hi"lo
            // skip the starting quote
            word_start++;

            // + 1 for null termination
            word_size = i - word_start + 1;

            // copy into buffer
            copy_comm(toRet, commands, word_start, word_size, comm_num);
            
            // advance to next command space
            comm_num++;

            // onto the next one 
            i++;
            word_start = i;

        } else if (word_start == i && ch == ' ') {
            // skip repeated spaces

            i++;
            word_start = i;

        } else if (ch == '|' || ch == '<' ) {
            // treat characters not allowing for file descriptors
            
            // case where we are already in the process of parsing a word,
            // finish it up then continue
            if (word_start != i) {

                // + 1 for null termination
                word_size = i - word_start + 1;

                // copy into buffer
                copy_comm(toRet, commands, word_start, word_size, comm_num);
                
                // advance to next command space
                comm_num++;

                word_start = i;
            }

            // +1 to include character i is on and +1 to include null term
            word_size = i - word_start + 2;

            // copy into buffer
            copy_comm(toRet, commands, word_start, word_size, comm_num);

            // advance to next command space
            comm_num++;

            // onto the next one 
            // -1 to ignore the null termination character
            i += word_size - 1;
            word_start = i;
 
            

        } else if (ch == '>') {
            // if have a redirection character, end the word now and treat it

            // case where we are already in the process of parsing a word,
            // finish it up then continue
            if (word_start != i) {

                // check if the word that is touching the special character is a
                // number, if so, treat it as a file descriptor. if it is a file
                // descriptor, don't seperate it from the special character
                if (!is_number(commands, word_start, i)) {
                    // + 1 for null termination
                    word_size = i - word_start + 1;

                    // copy into buffer
                    copy_comm(toRet, commands, word_start, word_size, comm_num);
                    
                    // advance to next command space
                    comm_num++;

                    word_start = i;
                } 
            }

            // +1 to include character i is on and +1 to include null term
            word_size = i - word_start + 2;

            // treat appending special character
            if (commands[i+1] == '>') {
                word_size += 1;
            } 

            // handle advanced redirection: duplicating file descriptor
            // see if fd is being duplicated, denoted by &
            if (commands[word_start + word_size - 1] == '&') {
                int fd_start = word_start + word_size;
                int fd_end = (int)(strchr(commands + fd_start, ' ') - commands);

                if (is_number(commands, fd_start, fd_end)) {
                    // +1 to include &
                    word_size += fd_end - fd_start + 1;
                }

            }

            // copy into buffer
            copy_comm(toRet, commands, word_start, word_size, comm_num);

            // advance to next command space
            comm_num++;

            // onto the next one 
            // -1 to ignore the null termination character
            i += word_size - 1;
            word_start = i;

        } else if (ch == ' ') {
            // + 1 for null termination
            word_size = i - word_start + 1;

            // copy into buffer
            copy_comm(toRet, commands, word_start, word_size, comm_num);
            
            // advance to next command space
            comm_num++;
            
            // onto the next one 
            i++;
            word_start = i;

        } else if (ch == '\0' || ch == '\n') {
            // printf("%c\n", commands[word_start]);
            if (commands[word_start] == '\0' || 
                commands[word_start] == ' '  ||
                commands[word_start] == '\n')
                break;

            // + 1 for null termination
            word_size = i - word_start + 1;

            // copy into buffer
            copy_comm(toRet, commands, word_start, word_size, comm_num);
            
            break;

        } else {
            i++;
        }

    }


    return toRet;
}

