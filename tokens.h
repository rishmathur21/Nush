/* This file is lecture notes from CS 3650, Fall 2018 */
/* Author: Nat Tuck */

#ifndef TOKENIZE_H
#define TOKENIZE_H

#include "svec.h"

char* tokenize_word(char* buf, int bufIndex, int tokenStart);

int space_case(svec* xs, char* buf, int bufIndex, int tokenStart);


int single_symbol_case(svec* xs, char* buf, int bufIndex, int tokenStart);
int double_symbol_case(svec* xs, char* buf, int bufIndex, int tokenStart);
svec* tokenize(char* buf);
void print_vector(svec* xs, int nn);


#endif
