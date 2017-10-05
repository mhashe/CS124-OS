#include <unistd.h> // read, write
#include <stdio.h> // printf
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "consts.h"
#include "com_parser.h"


int set_fn(struct command* cmd, char** pipe_tokens) {
    // Defaults, overridden if applicable
    cmd->input_fn = NULL;
    cmd->output_fn = NULL;
    cmd->error_fn = NULL;

    // Loop through all commands, look for <, >
    int idx = 0;
    while (pipe_tokens[idx] != NULL) {

        // Redirected input
        if (strcmp(pipe_tokens[idx], "<") == 0) {
            // We assume a properly formatted command (i.e., at most one <, >)
            cmd->input_fn = pipe_tokens[idx+1];

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
        if (strcmp(pipe_tokens[idx], ">") == 0) {
            // Same assumption as above
            cmd->output_fn = pipe_tokens[idx+1];

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


struct command* new_command(char** pipe_tokens) {
    struct command *cmd;
    cmd = (struct command *)calloc(1, sizeof(struct command));
    if (cmd == NULL) {
        fprintf(stderr, "Calloc failure for allocating command struct.\n");
        exit(1);
    }

    // Set redirected I/O, if applicable
    int set = set_fn(cmd, pipe_tokens);
    if (set == 1) {
        // Error with redirected I/O
        return NULL;
    }

    // Set up for execvp
    cmd->exec_fn = pipe_tokens[0];
    cmd->argv = pipe_tokens;
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
        fprintf(stderr, "Unexpected error in split_by_pipe_symbol!\n");
        fprintf(stderr, "(lower, upper) = (%d,%d)\n", lower, upper);
        exit(1);
    }

    // Store relevant commands in new array
    char** pipe_tokens = (char**)calloc(upper - lower + 2, sizeof(char*));
    if (pipe_tokens == NULL) {
        fprintf(stderr, "Calloc failure for allocating pipe_tokens array.\n");
        exit(1);
    }

    for (int i = lower; i <= upper; i++) {
        // Copy token into new array
        pipe_tokens[i-lower] = (char*)calloc(strlen(argv[i])+1, sizeof(char));
        if (pipe_tokens[i-lower] == NULL) {
            fprintf(stderr, "Calloc failure for allocating token.\n");
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
    if (cmd == NULL) {
        // Invalid I/O redirection - abort parsing, return to prompt.
        return NULL;
    }

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
        if (cmd == NULL) {
            // Invalid I/O redirection - abort parsing, return to prompt.
            return NULL;
        }
        n += 1;
    }

    return head_cmd;
}
