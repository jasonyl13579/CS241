/**
* Pied Piper Lab
* CS 241 - Spring 2018
*/

#include "pied_piper.h"
#include "utils.h"
#include <fcntl.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define TRIES 3
#define FD_READ 0
#define FD_WRITE 1
static int child_count = -1;
int pied_piper(int input_fd, int output_fd, char **executables) {
    // Your code goes here...
	int in = 0;
	if (input_fd != -1)
		in = input_fd;
	int out = 1;
	if (output_fd != -1)
		out = output_fd;
	//char* score = "1";
	//printf("%zd\n",sizeof(executables));
	char** commands = executables;
	size_t size = 0;
	while(commands[size] != NULL){
		size++;
		//printf("%s\n",*commands);
	}
	printf("s:%zd\n",size);
	pid_t *childs = malloc(size * sizeof(pid_t));
	int **fds = malloc((size+1) * sizeof(int*));
	int count = 0;	
	while(1)
	{
		fds[count] = malloc(2*sizeof(int));
		pipe(fds[count]);
		child_count ++;
		childs[count] = fork();
		
		if (childs[count] < 0)
		{
			printf("\nError in fork");
			return -1;
		}
		else if (childs[count]) //parent
		{
			if (count < (int)size-1) 
			{
				count++;
				continue;
			}
			
			//close all opened pipes
			for (int i = 0; i <= count; i++)
			{
				close(fds[i][FD_READ]);
				close(fds[i][FD_WRITE]);
			}
			
			for (size_t i = 0; i < size; i++)
				waitpid(childs[i], NULL, 0);
			//puts("2");
			return 0;			
		}
		else	//child
		{
			//printf("pid:%d,count:%d\n",getpid(),count);
			//printf("c:%d\n",child_count);
			if (count == 0)
				dup2(in,0);
			else
				dup2(fds[count-1][FD_READ], 0);
			
			if (count == (int)size-1)
				dup2(out,1);
			else
				dup2(fds[count][FD_WRITE],1);
							
				
			//close all other opened pipes
			for (int i = 0; i <= count; i++)
			{
				close(fds[i][FD_READ]);
				close(fds[i][FD_WRITE]);
			}	
			//if (count == 0) break;
			exec_command(executables[count]);		
			//if execv failed
		}
	}
/*	for (size_t i=0 ; i<size+1 ; i++){
		filedes[i] = malloc(2*sizeof(int));		
		pipe(filedes[i]);
		//printf("d1:%d\n",filedes[i][0]);
		//printf("d2:%d\n",filedes[i][1]);
	}
	char buf;
	filedes[0][0] = in;
	filedes[size][1] = out;
	for (size_t i=0 ; i<size+1 ; i++){
		//filedes[i] = malloc(2*sizeof(int));		
		//pipe(filedes[i]);
		printf("d1:%d\n",filedes[i][0]);
		printf("d2:%d\n",filedes[i][1]);
	}
	for (size_t i=0 ; i<size ; i++){
		
		childs[i] = fork();		
		if (childs[i] == 0){ // child i
			close(filedes[i][1]); //do read
			exec_command(executables[i]);
			while (read(filedes[i][0], &buf, 1) > 0)
				write(filedes[i+1][1], &buf, 1);
			close(filedes[i][0]);
			printf("pid1:%d\n",childs[i]);
		}else if(childs[i] >0){	//parent
			
			if (i == size-1){
				while (read(filedes[i][0], &buf, 1) > 0)
					write(filedes[i+1][1], &buf, 1);
				//close(filedes[i+1][1]);
			}else{
				//close(filedes[i][0]);
				//close(filedes[i][1]);
			}
			close(filedes[i][0]);
			close(filedes[i][1]);
			printf("pid2:%d\n",childs[i]);
			//int status;
			//waitpid(childs[i] , &status ,0);
		}
	}
	//close(filedes[3][1]);
	while (read(filedes[3][0], &buf, 1) > 0){}
	close(filedes[3][0]);
	//	write(STDOUT_FILENO, &buf, 1);
	for (size_t i=0 ; i<size ; i++){
		int status;
		waitpid(childs[i] , &status ,0);
	}*/
	
	//FILE *out = fdopen(output_fd, "w");
    return EXIT_SUCCESS;
}
