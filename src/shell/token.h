#ifndef TOKEN_H
#define TOKEN_H

/*
 * copy src[word_start:word_start+word_size] into dest[i] as a null term string
 */ 
void copy_comm(char** dest, char* src, int word_start, int word_size, int i);


/*
 * checks if a string[start_i:end_i] is a positive number
 *
 * Returns:
 *     1: if valid positive number
 *     0: otherwise
 * 
 */
int is_number(char* str, int start_i, int end_i);


/*
 * tokenizes a string for command parsing
 *
 * commands: 
 *     command to be parsed
 *
 * Returns:
 *     array of char pointers (strings) of tokenized values
 */
char** tokenize(char* commands);

#endif