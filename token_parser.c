#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "commands.h"
#include "buffer_tokenizer.h"
#include "svec.h"
#include "token_parser.h"
/*
 * Parsing Precedence:
 * "&"
 * "&&", "||"
 * "("
 * "|"
 * ">", "<"
 * ";"
 */
static const char* OPERATORS[8] = {"&", "&&", "||", ";", "|", "(", ">", "<"};

int
execute_shell_line(char* buffer)
{
		int status;
		svec* tokens;
		tokens = make_svec();

		// tokenize input stream
		// if not input continue
		if(!(tokenize(tokens, buffer))){

			// parse and execute tokens
			status = parse_execute_tokens(tokens);

			free(tokens);

			return status;
		}
		return 0;
}

int
parse_execute_tokens(svec* tokens)
{
		int ii, status, ff, jj;
		svec* f_half;
		svec* s_half;
	 	char*	file;
	 	char*	element;
		char** sargs;
		char** fargs;
		//execute operators by order of precedence
		for(ii = 0; ii < 8; ii++){

			for(jj = 0; jj < tokens->size; jj++){
				
				element = svec_get(tokens, jj);

				if(strcmp(OPERATORS[ii], element) == 0){
					switch (ii)
					{
							// 0 == "&" Background
							case 0:

								f_half = make_svec();
								s_half = make_svec();
								split_tokens(tokens, jj, f_half, s_half);

								status = background_cmd(f_half);

								//free malloced data structures 
								free_svec(f_half);
								free_svec(s_half);

								return status;

							// 1 == "&&" AND
							case 1:

								f_half = make_svec();
								s_half = make_svec();
								split_tokens(tokens, jj, f_half, s_half);
								
								status = parse_execute_tokens(f_half);
								
								if(status == 0){
									status = !(!status && !parse_execute_tokens(s_half));
								}
								
								//free malloced data structures 
								free_svec(f_half);
								free_svec(s_half);

								return status;

							// 2 == "||" OR
							case 2:

								f_half = make_svec();
								s_half = make_svec();
								split_tokens(tokens, jj, f_half, s_half);
								
								status = parse_execute_tokens(f_half);

								// check return status 
								if(status != 0){
									status = parse_execute_tokens(s_half);
								}

								//free malloced data structures 
								free_svec(f_half);
								free_svec(s_half);

								return status;

							// 3 == ";" Multiple commands
							case 3:

								f_half = make_svec();
								s_half = make_svec();
								split_tokens(tokens, jj, f_half, s_half);

								// evaluate both sides of semi colon individually
								status = !(!parse_execute_tokens(f_half) && !parse_execute_tokens(s_half));
								// free malloced data structures
								free_svec(f_half);
								free_svec(s_half);
								return status;

							// 4 == "|" Pipe
							case 4:
								//find preceding and succeeding commands to operator
								
								f_half = make_svec();
								s_half = make_svec();
								fargs = find_preceding_cmd(tokens, jj);
								
								split_tokens(tokens, jj, f_half, s_half);

								status = pipe_cmds(fargs, s_half);			
								free_svec(f_half);
								free_svec(s_half);
								
								return status;

							// 5 == "(" Sub-shell
							case 5:
								return subshell_cmd(svec_get(tokens, jj));
							
							// 6 == ">" Output Redirect
							case 6:
								// find preceding command to redirect output
								fargs = find_preceding_cmd(tokens, jj);
								// file is the token after output redirect
								file = svec_get(tokens, jj+1);
								
								status = redirect_output(fargs, file);
								
								return status;

							// 7 == "<" Input Redirect
							case 7:
								// find preceding command to redirect input
								fargs = find_preceding_cmd(tokens, jj);

								// file is token after redirect token
								file = svec_get(tokens, jj+1);

								status = redirect_input(fargs, file);
								
								return status;
						}
					}
				}
			}
			

		// Base Case for commands
		char** args;

		// the arguments must be NULL terminated so add one
		args = malloc(sizeof(char*) * (tokens->size + 1));

		// arguments are all remaining elements in svec
		for(ii = 0; ii < tokens->size; ii++){
			args[ii] = svec_get(tokens, ii);
		}
		// argument array must be null terminated;
		args[ii] = NULL;

		// check for builtin shell commands
		if (strcmp(args[0], "cd\0") == 0){
			return change_directory(args[1]);
		}
		else if (!(strcmp(args[0], "exit\0"))){
			exit(0);
		}

		status = execute_cmd(args, NULL, 0);

		return status;

}

/*
 * Splits tokens by given index into two string vectors.
 * Tokens: tokenized user input in string vector form
 * split: index to split at
 * f_part: string that will point to succeeding command contained in svec
 * s_part: string array that will point to the associated arguments in svec
 */
void
split_tokens(svec* tokens, int split, svec* f_part, svec* s_part)
{
		int ii;
		for(ii = 0; ii < split; ii++){
			svec_push_back(f_part, svec_get(tokens, ii));
		}

		ii = split + 1;
		for(; ii < tokens->size; ii++){
			svec_push_back(s_part, svec_get(tokens, ii));
		}
		
}

/*
 *	Check if given index of string vector is in the 
 *	operator array.
 */
int
check_operator(svec* tokens, int ii)
{
		int jj;
		//iterate over length of array
		for(jj = 0; jj < 8; jj++){
			if(!(strcmp(OPERATORS[jj], svec_get(tokens, ii)))){
				return 1;
			}
		}
		//return 0 if its not an operator
		return 0;
}


/*
 * Finds the preceding command and its arguments following a specific operator index.
 * Tokens: tokenized user input in string vector form
 * split: index of given operator
 * f_cmd: string that will point to preceding command contained in svec
 * fargs: string array that will point to the associated arguments in svec
 */
char**
find_preceding_cmd(svec* tokens, int split)
{

		char** fargs;
		int ii, pre_idx, op_idx;

		for(ii = split-1; ii >= 0; ii--){
			if ((check_operator(tokens, ii))) {
				// preceding cmd is index following first operator found
				pre_idx = ii + 1;
				break;
			}
			else if (ii == 0){
				// if none of the preceding strings are operators
				// then first element is preceding cmd
				pre_idx = 0;
				break;
			}
		}

		// allocate memory for argument string array
		fargs = malloc(sizeof(char*) * (split + 1 - pre_idx));
		// get elements of command
		ii = pre_idx;
		for(; ii < split; ii++){
			fargs[ii - pre_idx] = svec_get(tokens, ii);
		}
		// argument array must be null terminated
		fargs[ii - pre_idx] = NULL;

		return fargs;
}

/*
 * Finds the succeeding command and its arguments following a specific operator index.
 * Tokens: tokenized user input in string vector form
 * split: index of given operator
 * s_cmd: string that will point to succeeding command contained in svec
 * sargs: string array that will point to the associated arguments in svec
 */
char**
find_succeeding_cmd(svec* tokens, int split)
{

		char** sargs;
		int ii, kk, suc_idx, op_idx;
		for(ii = split + 1; ii < tokens->size; ii++){
			if ((check_operator(tokens, ii))) {
				// preceding cmd is index following first operator found
				suc_idx = ii - 1;
				break;
			}
			else if (ii == (tokens->size - 1)){
				// if none of the succeeding strings are operators
				// then first element is preceding cmd
				suc_idx = (tokens->size - 1);
				break;
			}
		}

		// allocate memory for argument string array
		sargs = malloc(sizeof(char*) * (suc_idx - split + 1));

		ii = 1;
		for(; ii <= (suc_idx - split); ii++){
			sargs[ii-1] = svec_get(tokens, split + ii);
		}
		// argument array must be null terminated
		sargs[ii-1] = NULL;

		return sargs;

}
