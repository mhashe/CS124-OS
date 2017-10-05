#ifndef TOKEN_H
#define TOKEN_H

void copy_comm(char** dest, char* src, int word_start, int word_size, int i);
int is_number(char* str, int start_i, int end_i);
char** tokenize(char* commands);

#endif