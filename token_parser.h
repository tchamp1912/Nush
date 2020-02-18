#include "svec.h"

#ifndef TOKEN_PARSER_H
#define TOKEN_PARSER_H

int
execute_shell_line(char* buffer);

int 
parse_execute_tokens(svec* tokens);

void
split_tokens(svec* tokens, int split, svec* f_part, svec* s_part);

char**
find_preceding_cmd(svec* tokens, int split);

char**
find_succeeding_cmd(svec* tokens, int split);

#endif
