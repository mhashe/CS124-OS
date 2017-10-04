#ifndef TOKEN_H
#define TOKEN_H

void copy_comm(char** dest, char* src, int word_start, int word_size, int i);
char** tokenize(char* commands);

#endif