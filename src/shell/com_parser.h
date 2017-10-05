#ifndef COM_PARSER_H
#define COM_PARSER_H


/* A command represents the fully tokenized & parsed command. It holds details 
including input/output/error redirection (if not null) as well as the command 
and its args for execution. Note that this level of io redirection be modified 
by the command itself (>, <). It can be represent a linked list. */
struct command {
    // Filename of executable (ie. "ls", "cat", etc)
    char *exec_fn;

    // List of arguments for executable, should always be null terminated w/ '0'
    char **argv;

    // Filenames for input/output/error redirection (null if doesn't exist) 
    char *input_fn;
    char *output_fn;
    char *error_fn;

    // If not null, next refers to the next command (whose out is this cmd's in)
    struct command *next; 
};

/* Parses a sequence of tokenized arguments (between-pipes, not including any) 
into a command struct, which it returns. */
struct command* new_command(char **argv);

/* Parse a sequence of tokenized arguments (including pipes) into a linked-list 
of commands using split_by_pipe_symbol. */
struct command* parse_to_chained_commands(char **argv);

/* Returns the arguments of argv before the first pipe symbol | is found. It 
modifies argv in place by cropping out the arguments before the first pipe. */
char** split_by_pipe_symbol(char **argv);

#endif
