#ifndef COM_PARSER_H
#define COM_PARSER_H

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
