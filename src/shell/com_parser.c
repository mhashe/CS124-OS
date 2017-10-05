#include <unistd.h> // read, write
#include <stdio.h> // printf
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "consts.h"
#include "com_parser.h"


/*
 * Sets input_fn and output_fn parameters, if applicable. Assumes commands
 * are well-structure, i.e. at most one redirected input file and one 
 * redirected output file.
 *
 * Inputs:
 *      cmd: A command struct
 *      pipe_commands: An array of tokenized commands
 *
 * Returns:
 *      1: Error opening/creating I/O files
 *      0: Normal execution
 * 
 */
int set_fn(struct command* cmd, char** pipe_commands) {
    // Defaults, overridden if applicable
    cmd->input_fn = NULL;
    cmd->output_fn = NULL;

    // Loop through all commands, look for <, >
    int idx = 0;
    while (pipe_commands[idx] != NULL) {

        // Redirected input
        if (strcmp(pipe_commands[idx], "<") == 0) {
            // We assume a properly formatted command (i.e., at most one <, >)
            cmd->input_fn = pipe_commands[idx+1];

            // Ensure that file actually exists, openable
            FILE *fp;
            fp = fopen(cmd->input_fn, "r");
            if (fp == NULL) {
                perror("Error with redirected input file");
                return 1;
            }
            if (fclose(fp) == EOF) {
                // Error with closing file.
                fprintf(stderr, "Error closing file for redirected file "
                    "input: %s\n", cmd->input_fn);
                exit(1);
            }
        }

        // Redirected output
        if (strcmp(pipe_commands[idx], ">") == 0) {
            // Same assumption as above
            cmd->output_fn = pipe_commands[idx+1];

            // Create file
            FILE *fp;
            fp = fopen(cmd->output_fn, "w");
            if (fp == NULL) {
                perror("Error in creating output file");
                return 1;
            }
            if (fclose(fp) == EOF) {
                // Error with closing file.
                fprintf(stderr, "Error closing file for redirected file "
                    "output: %s\n", cmd->output_fn);
                exit(1);
            }
        }

        idx += 1;
    }
    return 0;
}


/* Parses a sequence of tokenized arguments (between-pipes, not including any) 
into a command struct, which it returns. */
struct command* new_command(char** pipe_commands) {
    struct command *cmd;
    cmd = (struct command *)calloc(1, sizeof(struct command));

    int set = set_fn(cmd, pipe_commands);
    if (set == 1) {
        // Error with redirected I/O
        return NULL;
    }

    cmd->exec_fn = pipe_commands[0];
    cmd->argv = pipe_commands;
    cmd->next = NULL; // if there is a command piped from this, this will be changed
    return cmd;
}


/* Parse a sequence of tokenized arguments (including pipes) into a linked-list 
of commands using split_by_pipe_symbol. */
struct command* parse_to_chained_commands(char **argv) {
    struct command* head_cmd;
    struct command* cmd;
    char** pipe_commands;

    pipe_commands = split_by_pipe_symbol(argv, 0);
    if (pipe_commands == NULL) {
        // No valid command entered
        head_cmd = NULL;
        return head_cmd;
    }

    cmd = new_command(pipe_commands);
    if (cmd == NULL) {
        // Invalid I/O redirection
        return NULL;
    }

    head_cmd = cmd;
    
    int count = 1;
    while (1) {
        pipe_commands = split_by_pipe_symbol(argv, count);
        if (pipe_commands == NULL) {
            // No more commands
            break;
        }

        cmd->next = new_command(pipe_commands);
        cmd = cmd->next;

        if (cmd == NULL) {
            // Invalid I/O redirection
            return NULL;
        }
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
        pipe_commands[i-lower] = (char*)calloc(strlen(argv[i])+1, sizeof(char));
        strcpy(pipe_commands[i-lower], argv[i]);
    }
    pipe_commands[upper-lower+2] = NULL; // Terminate with a NULL

    return pipe_commands;
}
