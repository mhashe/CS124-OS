#include <unistd.h> // read, write
#include <stdio.h> // cout, cin
#include <stdlib.h>
#include <assert.h>

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
    struct command *cmd = new_command(split_by_pipe_symbol(argv));
    head_cmd = cmd;
    
    while (argv[0][0] != '\0') {
        cmd->next = new_command(split_by_pipe_symbol(argv));
        cmd = cmd->next;
    }
    return head_cmd;
}


/* Returns the arguments of argv before the first pipe symbol | is found. It 
modifies argv in place by cropping out the arguments before the first pipe. */
char** split_by_pipe_symbol(char **argv) {
    // TODO

    return argv;
}


/* Tests of com_parser go here. */
int main() {
    // TODO

    return 0;
}
