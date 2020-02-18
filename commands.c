#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "buffer_tokenizer.h"
#include "commands.h"
#include "svec.h"
#include "token_parser.h"

/*
 * This function was copied from Lecture Notes.
 */
void
check_rv(int rv)
{
    if (rv < 0) {
        perror("fail");
        exit(1);
    }
}

/*
 * Changes the working directory of current process.
 */
int 
change_directory(char* path)
{
	//builtin change directory
	return chdir(path);

}

/*
 * Generic shell execution command.
 * Takes in the command and its arguments.
 * Redirects the input/output to the desired file descriptor.
 * Returns status of execution in parent process.
 */
int
execute_cmd(char** args, int* fd, int i_o)
{
    int cpid;

    if ((cpid = fork())) {
        // parent process

				// wait for child return status
        int status;
        waitpid(cpid, &status, 0);        
				
				return status;
    }
    else {
        // child process
				// if a file descriptor not equal to stdout is passed
				// redirect output to it
				if (fd != NULL) {
						// close stdin or stdout
						close(i_o);
						// dup file descriptor in  its place
						dup(*fd);
						// close duplicate descriptor
						close(*fd);
				}

				// malloc individual arguments in child process
				int ii = 0;
				while (args[ii] != NULL){
					args[ii] = strdup(args[ii]);	
					ii++;
				}
				// The argv array for the child.
        // Terminated by a null pointer.
        execvp(args[0], args);

				// if there is an issue executing then the malloced
				// memory must be freed
				ii = 0;
				while (args[ii] != NULL){
					free(args[ii]);
					ii++;
				}

				exit(1000);
    }
}

/*
 * Redirects output of child process to a specified output file.
 */
int 
redirect_output(char** fargs, char* o_file)
{
		int output_d;
		// open file with proper write permissions
		output_d = open(o_file, O_CREAT | O_APPEND | O_WRONLY, 0644);
		// execute process with given file descriptor
		return execute_cmd(fargs, &output_d, 1);
}

/*
 * Replaces input of child process with a specified input file.
 */
int 
redirect_input(char** fargs, char* i_file)
{
		int input_d;

		// open file with proper read permissions
		input_d = open(i_file, O_RDONLY);
		// execute process with given file descriptor
		return execute_cmd(fargs, &input_d, 0);	
}

/*
 * Create two child processes.
 * Redirect the output of first process with input of the other.
 */
int 
pipe_cmds(char** fargs, svec* s_half)
{
    int fcpid, scpid, p_read, p_write, rv, status;

		// create pipe
		int pipe_fds[2];
    rv = pipe(pipe_fds);
    check_rv(rv);

    p_read  = pipe_fds[0];
    p_write = pipe_fds[1];

    if ((fcpid = fork())) {
			if((scpid = fork())) {
					// parent process
					// close pipes in parent process
					close(p_read);
					close(p_write);
					// check status of child processes
        	int fstatus;
       		waitpid(fcpid, &fstatus, 0);

        	int sstatus;
       		waitpid(scpid, &sstatus, 0);

					//return status of operation
					return !(!fstatus && !sstatus);
			}
			else {
					// second child process
					// close stdin
					close(0);
					// close write pipe in second process
					close(p_write);
					// dup p_read descriptor
					dup(p_read);
					// close p_read to manage open descriptors 
					close(p_read);
					
					status = parse_execute_tokens(s_half);

					free_svec(s_half);

					exit(status);
			}
    }
    else {
        // first child process
				// close stdout and p_read
				close(1);
				close(p_read);
				// dup p_write
				dup(p_write);
				// close p_write to manage open descriptors
				close(p_write);
				// exec first command
				// malloc individual arguments in child process
				int ii = 0;
				for (; ii < sizeof(fargs)/sizeof(char*); ii++)
					fargs[ii] = strdup(fargs[ii]);	
        
				// The argv array for the child.
        // Terminated by a null pointer.
        execvp(fargs[0], fargs);

				// if there is an issue executing then the malloced
				// memory must be freed
				for (; ii < sizeof(fargs)/sizeof(char*); ii++)
					free(fargs[ii]);

				exit(1000);
    }

}



int 
subshell_cmd(char* sub_cmd)
{
		int cpid, status;

		if((cpid = fork())){
			// parent process
			// wait for child return status
      waitpid(cpid, &status, 0);

			return status;
		
		}
		else{
			// return result of execution to parent
			return execute_shell_line(sub_cmd);
		}

}

/*
 * Forks and executes left side of operator "&" and returns 0 in parent process.
 */
int 
background_cmd(svec* tokens)
{
		int cpid, status;

		// return in parent process and terminate child process when done
		if((cpid = fork())){
			//parent process
			return 0;
		}
		else{
			// execute left side of background and terminate when done
			exit(parse_execute_tokens(tokens));
		}
}

