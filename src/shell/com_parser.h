#ifndef COM_PARSER_H
#define COM_PARSER_H


/*
 * A structure representing a fully tokenized and parsed command. Held in a 
 * linked list of piped commands.
 */
struct command {
    // Filename of executable (ie. "ls", "cat", etc)
    char *exec_fn;

    // List of arguments for executable, should always be null terminated w/ '0'
    char **argv;

    // Filenames for input/output/error redirection (null if doesn't exist) 
    char *input_fn;
    char *output_fn;
    char *error_fn;

    // Output/error append flags
    int out_a;
    int err_a;

    // If not null, next refers to the next command (whose out is this cmd's in)
    struct command *next; 
};


/*
 * Sets input_fn, output_fn, and error_fn parameters, if applicable. Assumes 
 * commands are well-structured, i.e. at most one redirected file of each type.
 *
 * Inputs:
 *     cmd: A command struct
 *     pipe_tokens: An array of tokens
 *
 * Returns:
 *     1: Error opening/creating I/O files
 *     0: Normal execution
 * 
 */
int set_fn(struct command* cmd, char** pipe_commands);


/*
 * Creates a single new command struct from a series of tokens.
 *
 * Inputs:
 *     pipe_tokens: An array of tokens
 *
 * Returns:
 *     NULL: Error with redirected I/O
 *     cmd: Normal execution; a command struct.
 * 
 */
struct command* new_command(char **argv);


/*
 * Return tokens between the nth and (n+1)st piping symbols (|).
 *
 * Inputs:
 *     argv: A list of tokens, potentially including piping symbols.
 *
 * Returns:
 *     NULL: The requested command cannot be found.
 *     pipe_tokens: The tokens corresponding to the requested command.
 * 
 */
char** split_by_pipe_symbol(char **argv, int n);


/*
 * Creates a linked list of command structs from a series of tokens, 
 * potentially including piping symbols (|).
 *
 * Inputs:
 *     argv: An array of tokens
 *
 * Returns:
 *     NULL: Invalid command
 *     head_cmd: The head of the linked list of commands
 * 
 */
struct command* parse_to_chained_commands(char **argv);

#endif
