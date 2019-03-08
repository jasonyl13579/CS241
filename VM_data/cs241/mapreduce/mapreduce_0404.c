/**
*  Lab
* CS 241 - Spring 2018
*/

#include "utils.h"
#include <alloca.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define FD_READ 0
#define FD_WRITE 1
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

void setup_fds(int new_stdin, int new_stdout);
void close_and_exec(char *exe, char *const *params);
pid_t start_reducer(char *reducer_exec, int in_fd, char *output_filename);
pid_t *read_input_chunked(char *filename, int *fds_to_write_to,
                          int num_mappers);
pid_t *start_mappers(char *mapper_exec, int num_mappers, int **read_mapper,
                     int write_reducer);
size_t count_lines(const char *filename);

void usage() {
    print_usage();
}

int main(int argc, char **argv) {
	if (argc != 6){
		print_usage();
		return -1;
	}
	int size = atoi(argv[5]);
	char* input_filename = argv[1];
    // Create an input pipe for each mapper.
	int **fd_to_mappers = malloc(size * sizeof(int*));
	for (int i=0 ; i<size ; i++){
		fd_to_mappers[i] = malloc(2*sizeof(int));
		pipe(fd_to_mappers[i]);
		descriptors_add(fd_to_mappers[i][0]);
		descriptors_add(fd_to_mappers[i][1]);
	}
    // Create one input pipe for the reducer.
	int fd_reducer[2];
	pipe(fd_reducer);
	descriptors_add(fd_reducer[0]);
	descriptors_add(fd_reducer[1]);
    // Open the output file.
	//int out = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT,
     //             S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	//descriptors_add(out);
    // Start a splitter process for each mapper.
	pid_t splitter_process[size];
	
	for (int i=0 ; i<size ; i++){
		splitter_process[i] = fork();
		if (splitter_process[i] < 0)
		{
				//printf("\nError in fork");
				return -1;
		}
		else if (splitter_process[i]) //parent
		{
			if (i < size-1) 
			{
				continue;
			}
		}
		else //child
		{
			setup_fds(0, fd_to_mappers[i][FD_WRITE]);
			char exec[30];
			sprintf(exec,"./splitter %s %d %d",input_filename,size,i);
			//printf("%s\n",exec);
			system(exec);
			exit(0);
		}
	}
	int status = 0;
	
	//system(exec);
    // Start all the mapper processes.
	pid_t *mapper_process;
	mapper_process = start_mappers(argv[3], size, fd_to_mappers, fd_reducer[1]);
	if (mapper_process == NULL) return -1;
	
    // Start the reducer process.
	pid_t reducer_process;
	reducer_process = start_reducer(argv[4], fd_reducer[0], argv[2]);
	if (reducer_process == -1) return -1;
	
    // Wait for the reducer to finish.
	descriptors_closeall();
	waitpid(reducer_process, &status, 0);
    // Print nonzero subprocess exit codes.
	//print_nonzero_exit_status(char *exec_name, int exit_status)
    // Count the number of lines in the output file.
	print_num_lines(argv[2]);
	// destroy
	descriptors_destroy();
	for (int i=0 ; i<size ; i++){
		free(fd_to_mappers[i]);
	}
	free(fd_to_mappers);
	free(mapper_process);
    return 0;
}
void setup_fds(int new_stdin, int new_stdout){
	dup2(new_stdin,0);
	dup2(new_stdout,1);
	descriptors_closeall();
}
pid_t *start_mappers(char *mapper_exec, int num_mappers, int **read_mapper, int write_reducer)
{
	pid_t *mapper_process = malloc(sizeof(pid_t)*num_mappers);
	for (int i=0 ; i<num_mappers ; i++){
		mapper_process[i] = fork();
		if (mapper_process[i] < 0)
		{
				//printf("\nError in fork");
				return NULL;
		}
		else if (mapper_process[i]) //parent
		{
			if (i < num_mappers-1) 
			{
				continue;
			}
		}
		else //child
		{
			setup_fds(read_mapper[i][FD_READ],write_reducer);
			system(mapper_exec);
			exit(0);
		}
	}
	return mapper_process;
}
pid_t start_reducer(char *reducer_exec, int in_fd, char *output_filename)
{
	int out = open(output_filename, O_WRONLY | O_TRUNC | O_CREAT,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	pid_t reducer_process;
	reducer_process = fork();
	if (reducer_process < 0)
	{
			//printf("\nError in fork");
			return -1;
	}
	else if (reducer_process) //parent
	{
			
	}
	else //child
	{
		setup_fds(in_fd,out);
		system(reducer_exec);
		close(out);
		exit(0);
	}
	return reducer_process;
}