#include "svec.h"

#ifndef BUFFER_TOKENIZER_H
#define BUFFER_TOKENIZER_H

char*
trim_leading_whitespace(char* buffer);

int
tokenize(svec* tokens, char* buffer);

char
parse_single_operator(char buffer_element);

int
check_double_operator(char op, char buffer_element);

#endif
