// for strchrnul
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
 * copy src[word_start:word_start+word_size] into dest[i] as a null term string
 */ 
void copy_comm(char** dest, char* src, int word_start, int word_size, int i) {
    dest[i] = (char*) malloc(word_size * sizeof(char));
    strncpy(dest[i], src + word_start, word_size);
    dest[i][word_size-1] = '\0';
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

        } else if (ch == '|' || ch == '<' || ch == '>') {
            // if have a redirection character, end the word now and treat it

            // case where we are already in the process of parsing a word,
            // finish it up then continue
            if (word_start != i) {
                // + 1 for null termination
                word_size = i - word_start + 1;

                // copy into buffer
                copy_comm(toRet, commands, word_start, word_size, comm_num);
                
                // advance to next command space
                comm_num++;
            }

            // special character and null termination

            // treat appending special character
            if (ch == '>' && commands[i+1] == '>') {
                word_size = 3;
            } else {
                word_size = 2;
            }

            // copy into buffer
            copy_comm(toRet, commands, i, word_size, comm_num);

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

        } else if (ch == '\0') {
            if (commands[word_start] == '\0')
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


// // for testing purposes
// // TODO remove
// int main() {
//     char* command = " echo bar abc def >> foo.txt";
//     char** comms = tokenize(command);

//     char* comm;
//     int i = 0;
//     while(1) {
//         comm = comms[i];
//         if (comm == NULL) {
//             break;
//         }
//         printf("%s\n", comm);
//         i++;

//     }


//     return 0;
// }