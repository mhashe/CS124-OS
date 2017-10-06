/*
 * The Breakfast Club, CS 124, Fall 2017
 * Vaibhav Anand, Nikhil Gupta, Michael Hashe
 *
 * This file contains methods to parse a tokenized command string into a useful
 * command struct for execution.
 */

#include <unistd.h> // read, write
#include <stdio.h> // printf
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "consts.h"
#include "com_parser.h"

// TODO: check if executable exists here or just fail at execution?
// grep "f" < Makefile is treating "<" as another arg
// do we really need error handling on redirecting output?
// change perror to fprinft(stderr, message)
// don't return 1/NULL when io error. just print it out. continue parsing command. (but we need the command to fail again in execution, so leave the error-causing stuff in there?)

void set_fn(struct command* cmd, char** pipe_tokens) {
    // Defaults, overridden if applicable
    cmd->input_fn = NULL;
    cmd->output_fn = NULL;
    cmd->error_fn = NULL;

    // Append flags; set for >> and 2>>
    cmd->out_a = 0;
    cmd->err_a = 0;

    // Loop through all commands, look for <, >
    int idx = 0;
    while (pipe_tokens[idx] != NULL) {

        // Redirected input
        if (strcmp(pipe_tokens[idx], "<") == 0) {
            // We assume a properly formatted command (i.e., at most one <, >)
            cmd->input_fn = pipe_tokens[idx+1];
        }

        // Redirected output
        if (strcmp(pipe_tokens[idx], ">") == 0) {
            // Same assumption as above
            cmd->output_fn = pipe_tokens[idx+1];
        }

        // Redirected output - append
        if (strcmp(pipe_tokens[idx], ">>") == 0) {
            // Same assumption as above
            cmd->output_fn = pipe_tokens[idx+1];

            // Set append flag
            cmd->out_a = 1;
        }

        // Redirected error
        if (strcmp(pipe_tokens[idx], "2>") == 0) {
            // Same assumption as above
            cmd->error_fn = pipe_tokens[idx+1];
        }

        // Redirected error
        if (strcmp(pipe_tokens[idx], "2>>") == 0) {
            // Same assumption as above
            cmd->error_fn = pipe_tokens[idx+1];

            // Set append flag
            cmd->err_a = 1;
        }

        idx += 1;
    }
    return;
}

char** parse_argv(char** argv) {
    char** real_args = (char**)calloc(MAX_LINE, sizeof(char*));
    if (real_args == NULL) {
        fprintf(stderr, "CALLOC FAILED in command parser.\n");
        exit(1);
    }

    int idx = 0;
    while(argv[idx] != NULL) {
        if (!strchr(argv[idx], '"')) {
            // This is not a quote; any >, < characters imply redirection
            if (strchr(argv[idx], '>') || strchr(argv[idx], '<')) {
                // Redirection.
                break;
            }
        }

        // +1 for null-terminated character
        real_args[idx] = (char*)calloc(strlen(argv[idx])+1,sizeof(char));
        if (real_args[idx] == NULL) {
            fprintf(stderr, "CALLOC FAILED in command parser.\n");
            exit(1);
        }

        strncpy(real_args[idx], argv[idx], strlen(argv[idx])+1);

        idx += 1;
    }

    return real_args;
}

struct command* new_command(char** pipe_tokens) {
    struct command *cmd;
    cmd = (struct command *) calloc(1, sizeof(struct command));
    if (cmd == NULL) {
        perror("Calloc failure for allocating command struct.");
        exit(1);
    }

    // Set redirected I/O, if applicable
    set_fn(cmd, pipe_tokens);

    // Set up for execvp
    cmd->exec_fn = pipe_tokens[0];
    cmd->argv = parse_argv(pipe_tokens);
    // cmd->argv = pipe_tokens;
    cmd->next = NULL; // Potentially changed by piping
    return cmd;
}


char** split_by_pipe_symbol(char **argv, int n) {
    char *comm;

    // First, find the indexes corresponding to this command
    int lower = -1;
    int upper = -1;

    // Start at beginning
    if (n == 0) {
        lower = 0;
    }

    // Pipe symbols passed thus far
    int pipes = 0;
    int idx = 1;
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
        perror("Unexpected error in split_by_pipe_symbol!");
        fprintf(stderr, "(lower, upper) = (%d,%d)\n", lower, upper);
        exit(1);
    }

    // Store relevant commands in new array
    char** pipe_tokens = (char**)calloc(upper - lower + 2, sizeof(char*));
    if (pipe_tokens == NULL) {
        perror("Calloc failure for allocating pipe_tokens array.");
        exit(1);
    }

    for (int i = lower; i <= upper; i++) {
        // Copy token into new array
        pipe_tokens[i-lower] = (char*)calloc(strlen(argv[i])+1, sizeof(char));
        if (pipe_tokens[i-lower] == NULL) {
            perror("Calloc failure for allocating token.");
            exit(1);
        }

        strcpy(pipe_tokens[i-lower], argv[i]);
    }
    // Terminate with a NULL
    pipe_tokens[upper-lower+2] = NULL;

    return pipe_tokens;
}


struct command* parse_to_chained_commands(char **argv) {
    struct command* head_cmd;
    struct command* cmd;
    char** pipe_commands;

    // Get tokens corresponding to 0th command
    pipe_commands = split_by_pipe_symbol(argv, 0);
    if (pipe_commands == NULL) {
        // No valid command entered
        head_cmd = NULL;
        return head_cmd;
    }

    // Create command struct from tokens
    cmd = new_command(pipe_commands);

    head_cmd = cmd;
    
    int n = 1;
    while (1) {
        // Find tokens corresponding to nth command
        pipe_commands = split_by_pipe_symbol(argv, n);
        if (pipe_commands == NULL) {
            // No more commands
            break;
        }

        // Create command struct from tokens
        cmd->next = new_command(pipe_commands);
        cmd = cmd->next;

        n += 1;
    }

    return head_cmd;
}
