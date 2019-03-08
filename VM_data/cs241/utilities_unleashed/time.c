/**
* Utilities Unleashed Lab
* CS 241 - Spring 2018
*/
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "format.h"

typedef struct timespec timespec;
int main(int argc, char *argv[]) {
	if (argc == 1){
		print_time_usage();
		return 1;
	}
	timespec* start_time =(timespec*) malloc(sizeof(timespec));
	clock_gettime(CLOCK_MONOTONIC, start_time);
	pid_t child = fork();
	if (child == -1) {
		print_fork_failed();
		return EXIT_FAILURE;
	}
	if (child) { /* I have a child! */
		int status;
		waitpid(child , &status ,0);
		if (status)return 1;
		timespec* end_time = (timespec*) malloc(sizeof(timespec));
		clock_gettime(CLOCK_MONOTONIC, end_time);
		//printf("%ld\n",end_time - start_time);
		display_results(argv , ((double)(end_time->tv_sec - start_time->tv_sec)+(double)(end_time->tv_nsec-start_time->tv_nsec)/1000000000));
		free(end_time); free(start_time);
		return EXIT_SUCCESS;	
	} else { /* I am the child */
		// Other versions of exec pass in arguments as arrays
		// Remember first arg is the program name
		// Last arg must be a char pointer to NULL
		char** array = (char**)malloc(sizeof(char*)*argc);
		for (int i=0;i<argc;i++){
			if (i != argc-1){
				array[i] = (char*)malloc(sizeof(char)*(int)strlen(argv[i+1])+1);
				array[i] = argv[i+1];
				//printf("%s\n",array[i]);
			}else array[i] = NULL;
		}
		execvp(argv[1], array);
		//execlp("ls", "ls",NULL);
		print_exec_failed();
		// If we get to this line, something went wrong!
	  }

  
    return 0;
}
