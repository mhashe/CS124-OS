#include <unistd.h> // read, write
#include <stdio.h> // printf
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "consts.h"
#include "com_parser.h"

/*
 * Contains the command struct, which is parsed from a sequence of tokens, and 
 * the functions for doing said parsing.
 */


/* Parses a sequence of tokenized arguments (between-pipes, not including any) 
into a command struct, which it returns. */
struct command* new_command(char **argv) {
    struct command *cmd;
    cmd = (struct command *) malloc(sizeof(struct command));
    cmd->input_fn = NULL;
    cmd->output_fn = NULL;

    /* TODO:
        Parse from argvs the filename for input/output/error redirection if they exist.
        This should be done in a seperate function.
        This function should check that the files exist. Else, they should 
        either throw an error or return an error value.? 
        (If none are found, the default for cmd->input and cmd->ouput are null)
    */

    cmd->exec_fn = argv[0];
    cmd->argv = (argv + 1);
    cmd->next = NULL; // if there is a command piped from this, this will be changed
    return cmd;
}


/* Parse a sequence of tokenized arguments (including pipes) into a linked-list 
of commands using split_by_pipe_symbol. */
struct command* parse_to_chained_commands(char **argv) {
    struct command *head_cmd;
    struct command *cmd = new_command(split_by_pipe_symbol(argv, 0));
    head_cmd = cmd;
    
    int count = 1;
    while (argv[0][0] != '\0') {
        cmd->next = new_command(split_by_pipe_symbol(argv, count));
        cmd = cmd->next;
        count += 1;
    }
    return head_cmd;
}


/* Returns the arguments of argv between the nth and the (n+1)st pipe symbols. */
char** split_by_pipe_symbol(char **argv, int n) {
    char *comm;

    // First, find the indexes corresponding to this command
    int lower = -1;
    int upper = -1;

    // Start at beginning
    if (n == 0) {
        lower = 0;
    }

    int idx = 1;
    int pipes = 0; // Pipe symbols passed thus far
    while(1) {
        comm = argv[idx];

        // Reached end of array; do more later
        if (comm == NULL) {
            upper = idx - 1;
            break;
        }

        // Check if pipe symbol
        if (strcmp("|", comm) == 0) {
            pipes += 1;

            // Start after this token
            if (pipes == n) {
                lower = idx + 1;
            }

            // Ends before this token
            if (pipes == n + 1) {
                upper = idx - 1;
                break;
            }
        }

        idx += 1;
    }

    // Ensure we found a valid command, i.e. n <= pipes
    if (n > pipes) {
        return NULL;
    }

    // Sanity check
    if ((lower == -1) || (upper == -1)){
        // Change stderr to some file descriptor?
        fprintf(stderr, "Unexpected error in split_by_pipe_symbol!\n");
        fprintf(stderr, "(lower, upper) = (%d,%d)\n", lower, upper);
        exit(1);
    }
        // free(com0);
    // Store relevant commands in new file
    char** pipe_commands = (char**)calloc(upper - lower + 2, sizeof(char*));

    for (int i = lower; i <= upper; i++) {
        pipe_commands[i-lower] = (char*)calloc(strlen(argv[1]), sizeof(char));
        strcpy(pipe_commands[i-lower], argv[i]);
    }
    pipe_commands[upper-lower+2] = NULL; // Terminate with a NULL

    return pipe_commands;
}
