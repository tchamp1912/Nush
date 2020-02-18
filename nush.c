#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "token_parser.h"


int
main(int argc, char* argv[])
{
    char buffer[256];
		int size = 256;
		FILE* fd;

    if (argc == 1) {
				while(1){
					//begin reading in commands
        	printf("nush$ ");
        	fflush(stdout);

        	if(fgets(buffer, 256, stdin) == 0) break;

					execute_shell_line(buffer);
				}
    }
		// if there are two arguments the second is a shell script
		// read in shell script and execute each line
    else {
			// check if it is a valid file
    	if ((fd = fopen(argv[1], "r")) == NULL) {
        printf("Error opening shell script!");
        // Program exits if file pointer returns NULL.
        exit(1);
    	}

			while ((fgets(buffer, size, fd)) != 0){
    			execute_shell_line(buffer);
			}
			//close shell script
    	fclose(fd);

    }

    return 0;
}
