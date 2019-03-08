/**
* Shell Lab
* CS 241 - Spring 2018
*/

#include "format.h"
#include "shell.h"
#include "vector.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h> 
#include <sys/wait.h>
extern char ** environ;
typedef struct process {
    char *command;
    char *status;
    pid_t pid;
} process;
void command_execute( char* );
void cleanup(int );
void handleInt(int );
bool background(size_t , char** );
static vector *history_vector;
static process* processes;
static int process_number = 0;
static int exit_flag = 0;
static int fexit_flag = 1;
static FILE *command_file ;
int shell(int argc, char *argv[]) {
    // TODO: This is the entry point for your shell.
	char* history_file_name = NULL;
	char* command_file_name = NULL;
	int h = 0;
	int f = 0;
	
	int opt;
	FILE * history;
	
	if (argc == 2) {
		print_usage();
		exit(0);
	}
	history_vector = string_vector_create();
	process_number++;
	process pro;
	processes = (process*)realloc(processes,process_number * sizeof(process));		
	pro.command = "shell";
	pro.status = STATUS_RUNNING;
	pro.pid = getpid();
	processes[process_number-1] = pro;
	signal(SIGINT, handleInt); 
	while ((opt = getopt(argc, argv, "h:f:")) != -1) {
		
        switch (opt) {
        case 'h':
            history_file_name = strdup(optarg);
			h = 1;
            break;
        case 'f':
            command_file_name = strdup(optarg);
		//	printf("f:%s\n",command_file_name);
            f = 1;
            break;
        default: /* '?' */
            print_usage();
            exit(EXIT_FAILURE);
        }
    }
	if (h == 1){
		
		history = fopen(history_file_name, "r+" );
		if (history == NULL){
			print_history_file_error();
		}else{
			char *buffer = NULL;
			size_t size = 0;
			ssize_t chars ;
			while((chars = getline(&buffer, &size, history))!= -1){
				// Discard newline character if it is present,
				//printf("%zd\n",chars);
				if (chars > 0 && buffer[chars-1] == '\n') 
					buffer[chars-1] = '\0';
				vector_push_back(history_vector,buffer);
			}
			free(buffer);
		}
	}
	if (f == 1){
	//	puts("test");
		command_file = fopen(command_file_name, "r+" );
		if (command_file == NULL){		
			print_script_file_error();
		}else{
			char *buffer = NULL;
			size_t size = 0;
			ssize_t chars ;
			signal(SIGCHLD, cleanup); 
			while((chars = getline(&buffer, &size, command_file))!= -1){
				// Discard newline character if it is present,
				if (chars > 0 && buffer[chars-1] == '\n') 
					buffer[chars-1] = '\0';
			//	printf("test:%s\n",buffer);
				command_execute(buffer);
			}
			while(process_number != 1){sleep(1);}
			free(buffer);
			fclose(command_file);
		}
	}else{
		//read from stdin
		char *buffer = NULL;
		size_t size = 0;
		ssize_t chars ;
		signal(SIGCHLD, cleanup); 
		while( exit_flag == 0 && (chars = getline(&buffer, &size, stdin))!= -1 ){
				// Discard newline character if it is present,
			if (chars > 0) 
				buffer[chars-1] = '\0';
			//printf("%s%zd\n",buffer,chars);
			command_execute(buffer);
			/*for (int i=0 ; i<process_number; i++)
			print_process_info(processes[i].status,processes[i].pid,processes[i].command);*/
		}
		free(buffer);
	}
	
	//free
	for (int i=1; i<process_number; i++)
		kill(processes[i].pid, SIGTERM);
	//free(processes);
	if (command_file_name != NULL)free(command_file_name);
	if (history_file_name != NULL){
		fseek(history,0,SEEK_SET);
		for (size_t i=0; i<vector_size(history_vector); i++)
			fprintf(history,"%s\n",vector_get(history_vector,i));
		free(history_file_name);
	}
	if (history != NULL) fclose(history);
	vector_destroy(history_vector);
	free(processes);
    return 0;
}

void command_execute( char* command_input )
{
	char **tokens;
	size_t numtokens;
	char * path;
	bool save = true;
	tokens = strsplit(command_input, " ", &numtokens);
	if((path = getcwd(NULL, 0)) == NULL)  {  
		perror("getcwd error");  
	}  
	else {  			
		print_prompt(path, getpid()); 
		print_command(command_input);	
	}  
		
	if (strcmp(tokens[0] ,"cd" ) == 0){
		if (chdir(tokens[1])== -1){
			print_no_directory(tokens[1]);
		}
	}
	else if (strcmp(tokens[0] ,"!history" ) == 0){
		save = false;
		int index = 0;
		VECTOR_FOR_EACH(history_vector, elem, {
			if (_it != _iend){
				print_history_line(index , elem);
				index++;
			}
		});
	}
	else if (tokens[0][0] == '#'){
		save = false;
		size_t n = 0;
		//printf("%lu\n",strlen(tokens[0]));
		for(size_t i=1 ; i < strlen(tokens[0]) ; i++){
			n = (tokens[0][i] - '0' )+ n*10;
			//printf("%c\n",tokens[0][i]);
		}
		//printf("%d\n",n);
		if (n >= vector_size(history_vector)){
			print_invalid_index();
		}else{
			char* new_command = vector_get(history_vector,n);
			command_execute(new_command);
		}
	}
	else if (tokens[0][0] == '!'){
		save = false;
		int j;
		for (j = vector_size(history_vector)-1; j >= 0 ; j--){
			bool same = true;
			for(size_t i=1 ; i < strlen(tokens[0]) ; i++){
				char* temp = vector_get(history_vector,j);
				if (strlen(temp) < strlen(tokens[0])-1) continue;
				if (temp[i-1] != tokens[0][i]) same = false;
			}
			if (same)break;
		}
		if (j == -1)print_no_history_match();
		else{
			char* new_command = vector_get(history_vector,j);
			command_execute(new_command);
		}
	}
	else if (strcmp(tokens[0] ,"exit" ) == 0){
		save = false;
		exit_flag = 1;
	}
	else if (strcmp(tokens[0] ,"ps" ) == 0){
		save = false;
		for (int i=0 ; i<process_number; i++)
			print_process_info(processes[i].status,processes[i].pid,processes[i].command);
	}
	else if (strcmp(tokens[0] ,"kill" ) == 0){
		save = false;
		if (numtokens==1)print_invalid_command(command_input);
		pid_t child = (pid_t)atoi(tokens[1]);
		if (kill(child, SIGTERM) == -1)
			print_no_process_found(child);
		else{
			for (int i=0;i<process_number;i++){
				if (processes[i].pid == child){
					print_killed_process(child,processes[i].command);
				}
			}
			
		}
	}
	else if (strcmp(tokens[0] ,"stop" ) == 0){
		save = false;
		if (numtokens==1)print_invalid_command(command_input);
		pid_t child = (pid_t)atoi(tokens[1]);
        if (kill(child, SIGTSTP) == -1)      
		    print_no_process_found(child);
		else{
            
			for (int i=0;i<process_number;i++){
				if (processes[i].pid == child){
					print_stopped_process(child,processes[i].command);
					processes[i].status = STATUS_STOPPED ;
				}
			}
		}
	}
	else if (strcmp(tokens[0] ,"cont" ) == 0){
		save = false;
		if (numtokens==1)print_invalid_command(command_input);

		pid_t child = (pid_t)atoi(tokens[1]);
        if (kill(child, SIGCONT) == -1)
            print_no_process_found(child);
		else{
			for (int i=0;i<process_number;i++){
				if (processes[i].pid == child){
					processes[i].status = STATUS_RUNNING ;
				}
			}
		}

	}
	else if (!background( numtokens, tokens)){
		//fork without &
	//	puts("without &");
		size_t index;
		int special = 0;
		char** newtokens1 = NULL;
		char** newtokens2 = NULL;
		size_t numtokens1;
		size_t numtokens2;
		for (index=0 ; index<numtokens ; index++){
			if (strcmp(tokens[index], "&&") == 0){
				special = 1;
				break;
			}else if (strcmp(tokens[index], "||") == 0){
				special = 2;
				break;
			}else if (tokens[index][strlen(tokens[index])-1] == ';'){
				if (tokens[index][strlen(tokens[index])-2] != '\\'){
					special = 3;
					index++;
					break;
				}
			}
		}
		
		newtokens1 = (char**)calloc(index+1, sizeof(char *));
		numtokens1 = index;
		for (size_t i=0 ; i<index ; i++){
			newtokens1[i] = strdup(tokens[i]);
			// escape
			size_t j1=0;
			for (size_t j=0 ; j<strlen(tokens[i]);j++){
				if (newtokens1[i][j] == '\\' && j != strlen(tokens[i])-1){
					if (newtokens1[i][j+1] == '|' || newtokens1[i][j+1] == '&'||newtokens1[i][j+1] == ';' ){
						newtokens1[i][j1] = tokens[i][j+1];
						j++; 
					}
				}
				j1++;
			}
			newtokens1[i][j1] = '\0';
			// escape
			//printf("%s\n",newtokens1[i]);
		}
		newtokens1[index] = NULL;
		if (special == 3){
			newtokens1[index-1][strlen(newtokens1[index-1])-1] = '\0';	
			index--;			
		}
		if (special == 1 || special == 2 || special == 3){
			if ( numtokens-index-1 == 0 ) {
				print_invalid_command(command_input);
				return;
			}
			newtokens2 = calloc(numtokens-index, sizeof(char *));
			numtokens2 = numtokens-index-1;
			for (size_t i=0 ; i<numtokens-index-1 ; i++){
				newtokens2[i] = strdup(tokens[index+i+1]);
				// escape
				size_t j1=0;
				for (size_t j=0 ; j<strlen(tokens[index+i+1]);j++){
					if (newtokens2[i][j] == '\\' && j != strlen(tokens[index+i+1])-1){
						if (newtokens2[i][j+1] == '|' || newtokens2[i][j+1] == '&'||newtokens2[i][j+1] == ';' ){
							newtokens2[i][j1] = tokens[index+i+1][j+1];
							j++; 
						}
					}
					j1++;
				}
				newtokens2[i][j1] = '\0';
				// escape
			}
			newtokens2[numtokens2] = NULL;
		}
		/*if (special == 3) {
			newtokens1[index-1][strlen(newtokens1[index-1])-1] = '\0';
			if ( numtokens-index == 0 ) {
				print_invalid_command(command_input);
				return;
			}
			newtokens2 = calloc(numtokens-index+1, sizeof(char *));
			numtokens2 = numtokens-index;
			for (size_t i=0 ; i<numtokens-index ; i++){
				newtokens2[i] = strdup(tokens[index+i]);
			}
			newtokens2[numtokens2] = NULL;
		}*/
		fflush(stdout);
		if (command_file !=NULL) fflush(command_file);
		pid_t child = fork();		
		if (child == -1) {
			print_fork_failed();
			exit(1);
			//return EXIT_FAILURE;
		}
		else if (child){
			//parents
			int status;
		
			waitpid(child , &status ,0);
			free_args(newtokens1);
			
			if ((special == 1 && status == 0) || (special == 2 && status != 0) || special == 3){
				fflush(stdout);
				if (command_file !=NULL) fflush(command_file);
				pid_t child2 = fork();		
				if (child2 == -1) {
					print_fork_failed();
					//return EXIT_FAILURE;
					exit(1);
				}
				else if (child2){
					waitpid(child2 , &status ,0);
					
				}else{
					//child2
					process_number++;
					process pro;
					processes = (process*)realloc(processes,process_number * sizeof(process));		
					pro.command = command_input;
					pro.status = STATUS_RUNNING;
					pro.pid = child;
					
					processes[process_number-1] = pro;
					execvp(newtokens2[0] , newtokens2);
					char* temp = strdup(newtokens2[0]);
					if (numtokens2 > 1)strcat(temp," ");
					for (size_t i=1 ;i<numtokens2;i++){
						strcat(temp,newtokens2[i]);
					}
					print_exec_failed(temp);
					free(temp);
					exit(1);
				}	
		//		free_args(newtokens2);				
			}		
			if (special == 1 || special == 2 || special == 3)
		//	if((special == 1 && status != 0) || (special == 2 && status == 0))
				free_args(newtokens2);
		}else{
			//child
			process_number++;
			process pro;
			processes = (process*)realloc(processes,process_number * sizeof(process));		
			pro.command = command_input;
			pro.status = STATUS_RUNNING;
			pro.pid = child;
			processes[process_number-1] = pro;
			
			print_command_executed(getpid());
			execvp(newtokens1[0] , newtokens1);
			char* temp = strdup(newtokens1[0]);
			if (numtokens1 > 1)strcat(temp," ");
			for (size_t i=1 ;i<numtokens1;i++){
				strcat(temp,newtokens1[i]);
			}
			print_exec_failed(temp);
			free(temp);
			exit(1);
		}
		//end fork without &
	}
	else {
		// fork at with & (backgroud)
	//	printf("background\n");
		char** newtokens1 = NULL;
		size_t numtokens1;
		fexit_flag = 0;
		numtokens1 = numtokens;
		newtokens1 = (char**)calloc(numtokens1+1, sizeof(char *));
		if (numtokens == 2 && strcmp(tokens[1] ,"&") == 0) {
			newtokens1[0] = strdup(tokens[0]);
			numtokens1--;
		}
		if (numtokens == 3 && strcmp(tokens[2] ,"&") == 0) {
			newtokens1[0] = strdup(tokens[0]);
			newtokens1[1] = strdup(tokens[1]);
			numtokens1--;
		}
		if  (tokens[0][strlen(tokens[0])-1] == '&') {
			newtokens1[0] = strdup(tokens[0]);
			newtokens1[0][strlen(tokens[0])-1] ='\0';
		}
		if  (numtokens == 2 && tokens[1][strlen(tokens[1])-1] == '&') {
			newtokens1[0] = strdup(tokens[0]);
			newtokens1[1] = strdup(tokens[1]);
			newtokens1[1][strlen(tokens[1])-1] ='\0';
		}
		newtokens1[numtokens1] = NULL;

		fflush(stdout);		
		if (command_file !=NULL) fflush(command_file);
		pid_t child = fork();		
		if (child == -1) {
			print_fork_failed();
			//return EXIT_FAILURE;
			free(newtokens1);
			exit(1);
		}
		else if (child){
			//parents
			//int status;
			if (setpgid(child, child) == -1) {
				print_setpgid_failed();
				exit(1);
			}
			
			process_number++;
			process pro;
			processes = (process*)realloc(processes,process_number * sizeof(process));	
			
			pro.command = command_input;
			//usleep(100);
			pro.status = STATUS_RUNNING;
			pro.pid = child;
			//printf("command:%s\n",pro.command);
			processes[process_number-1] = pro;
			//waitpid(child , &status ,0);
		}else{
			//child
			print_command_executed(getpid());
			execvp(newtokens1[0] , newtokens1);
			print_exec_failed(command_input);
			exit(1);
		}
		for (size_t i=0 ; i<numtokens1; i++)
			free(newtokens1[i]);
		free(newtokens1);
	}
	if (save) vector_push_back(history_vector,command_input);
	free_args(tokens);	
	free(path);
	
}
void cleanup(int signal) {
	int status;
	//printf("signal:%d\n",signal);
	pid_t child = 0;
	while ((child = waitpid((pid_t) (-1), &status, WNOHANG))> 0) {
		//if (WIFEXITED(status)) printf("exit-");
		//printf("pid:%d\n",child);
		int i = 0;
		for (i=0; i<process_number;i++){
			if (processes[i].pid == child) break;
		}
		//printf("pid:%d\n",child);
		for (int j=i; j<process_number-1;j++){
			 processes[j] = processes[j+1];
		}
		process_number--;
		if (process_number == 1) fexit_flag = 1;
	//	printf("num:%d\n",process_number);
		fflush(stdin);
	}
	
}
void handleInt(int signal) {
	//int status;
	//printf("signal:%d\n",signal);
	if ( process_number == 2) kill(processes[1].pid, SIGTERM);
	//pid_t child = 0;
	//kill(getpid(), SIGTERM);
	
}
bool background(size_t numtokens, char** tokens){
	if (numtokens == 2){
		if (strcmp(tokens[1] ,"&") == 0) return true;
		else if (tokens[1][strlen(tokens[1])-1] == '&' && tokens[1][strlen(tokens[1])-2] != '\\') return true;
	}
	else if (numtokens == 3){
		if (strcmp(tokens[2] ,"&") == 0) return true;
	}
	else if (numtokens == 1){
		if (tokens[0][strlen(tokens[0])-1] == '&' && tokens[0][strlen(tokens[0])-2] != '\\' ) return true;
	}
	return false;
}