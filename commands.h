#include "svec.h"

#ifndef COMMANDS_H
#define COMMANDS_H

int 
change_directory(char* path);

int 
redirect_output(char** fargs, char* o_file);

int 
redirect_input(char** fargs, char* i_file);

int 
pipe_cmds(char** fargs, svec* s_half);

int 
subshell_cmd(char* sub_cmd);

int 
background_cmd(svec* tokens);

int 
execute_cmd(char** args, int* fd, int i_o);

#endif
