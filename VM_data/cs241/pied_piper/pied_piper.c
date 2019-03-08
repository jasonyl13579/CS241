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
static char buf[1000];
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
	//printf("s:%zd\n",size);
	pid_t *childs = malloc(size * sizeof(pid_t));
	int **fds = malloc((size+1) * sizeof(int*));
	int fd_error[2];
	for (size_t i = 0; i <= size; i++){
		fds[i] = malloc(2*sizeof(int));
		//fd_error[i] = malloc(2*sizeof(int));
	}
	int count = 0;	
	int try = TRIES;
	int end = 0;
	bool restart = false;
	int stdin_copy = dup(0);
	int stdout_copy = dup(1);
	failure_information *info = malloc ((size+1) *sizeof(failure_information));
	while(try && !end){
		//reset_file(input_fd);
		//reset_file(output_fd);
		restart = false;
		try--;
		//pipe(fd_error);
		pipe2(fd_error,O_CLOEXEC);
		
		count = 0;		
		dup2(stdin_copy,0);
		dup2(stdout_copy,1);
		//puts("try");
		while(1)
		{	
			pipe(fds[count]);		
			childs[count] = fork();
			if (childs[count] < 0)
			{
				//printf("\nError in fork");
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
				close(fd_error[1]);
				//close(0);
				//close(in);
				for (int i = 0; i <= count; i++)
				{
					close(fds[i][FD_READ]);
					close(fds[i][FD_WRITE]);
					//close(fd_error[i][FD_READ]);
					//close(fd_error[i][FD_WRITE]);
				}
				int status = 0;
				for (size_t i = 0; i < size; i++){
					waitpid(childs[i], &status, 0);
					//printf("status:%d\n",status);
				//	if (try == 0){
						info[i].command = executables[i];
						info[i].status = status;
						info[i].error_message = "\0";
				//	}
		
					if (status != 0) {
						//close(1);
						int index = 0;
						char b;
						while(read(fd_error[0], &b, 1) > 0){
							buf[index] = b;
							index++;
							//printf("%c\n",b);
							if (b == '\n') break;
						}
						//printf("XD\n");
						buf[index] = '\0';					
						info[i].error_message = buf;
						restart = true;
					}
				}
				//return 0;	
				if (restart) {
					count = 0;
					break;
				}
				end = 1;
				break;
			}
			else	//child
			{
				//printf("pid:%d,count:%d\n",getpid(),count);
				//printf("c:%d\n",child_count);
				//dup2(fd_error[1],2);
				close(fd_error[0]);
				if (try == 0){
					dup2(fd_error[1],2);
				}
				if (count == 0){
					if(dup2(in,0) == -1) exit(-1); //fprintf(stderr,"dup2 failed\n");
				}
				else{
					dup2(fds[count-1][FD_READ], 0);	
				}					
				if (count == (int)size-1){
					if(dup2(out,1) == -1) exit(-1); //fprintf(stderr,"dup2 failed\n");
					//dup2(out,1);
				}
				else{
					dup2(fds[count][FD_WRITE],1);
					//dup2(fds[count][FD_WRITE],2);
				}
								
					
				//close all other opened pipes
				for (int i = 0; i <= count; i++)
				{
					close(fds[i][FD_READ]);
					close(fds[i][FD_WRITE]);
				}	
				//if (count == 0) break;
				//fprintf(stderr,"test\n");
				
				if (exec_command(executables[count]) == -1)	{	
				//if execv failed
					//fprintf(stderr,"test");
					//close(0);
					//close(1);
					close(fd_error[1]);
					exit(-1);
				}
				exit(0);
			}
		}
	}
	//printf("try:%d\n",try);
	if (try == 0){
		print_failure_report(info,size);
	}
		
	for (size_t i = 0; i <= size; i++){
		free(fds[i]);
		//destroy_failure(&info[i]);
	}
	free(info);
	free(fds);
	free(childs);
	
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
	if (try == 0)
		return EXIT_OUT_OF_RETRIES;
	else
		return EXIT_SUCCESS;
}
