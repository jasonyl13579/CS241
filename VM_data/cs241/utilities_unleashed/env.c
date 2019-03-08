/**
* Utilities Unleashed Lab
* CS 241 - Spring 2018
*/
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "format.h"
#include <ctype.h>
#include <time.h>
extern char ** environ;
struct env_pairs{
	char* key;
	char** values;
	int numtokens;
};
typedef struct env_pairs env_pairs;
char **strsplit(const char*, const char*, size_t*);
int main(int argc, char *argv[]) {
	if (argc < 3){
		print_env_usage();
		return 1;
	}
	
	size_t n = 1;
	int count = 1;
	if (!strcmp(argv[1] , "-n")){
		if (isdigit(*argv[2])){
			n = atoi(argv[2]);
			count += 2;
		}else{
			print_env_usage();
			return 1;
		}
	}
    char **tokens;
    size_t numtokens;
	env_pairs *pairs;
	int numpairs = 0;
	pairs = (env_pairs*)malloc(sizeof(env_pairs)*numpairs);
	int pass_n_test = 0;
	while(argv[count]){
		if (!strcmp(argv[count] , "--"))break;
		numpairs ++;
		pairs = (env_pairs*)realloc(pairs,sizeof(env_pairs)*numpairs);
		 tokens = strsplit(argv[count++], ", \t\n=", &numtokens);
		 if (numtokens!= n+1 && numtokens!= 2) {
		//	printf("%zu:%zu\n",numtokens,n);
			pass_n_test = 0;
		 }
		 if (numtokens == n+1) pass_n_test = 1;
		 pairs[numpairs-1].numtokens = numtokens -1;
		 pairs[numpairs-1].key = tokens[0];
		 pairs[numpairs-1].values = tokens;
		 for (size_t i = 0; i < numtokens; i++) {
    //        printf("    token: \"%s\"\n", tokens[i]);
          //  free(tokens[i]);
        }
       // if (tokens != NULL)
         //   free(tokens);
		
	}
	if (!pass_n_test || argv[count] == NULL){
		print_env_usage();
		return 1;
	}
	count ++;
	char* cmd = argv[count++];
	if (cmd == NULL){
		print_env_usage();
		return 1;
	}
	//printf("%s\n",cmd);
	int args_count = 0;
	while(argv[count]){
		args_count ++; count++;
	}
	//printf("count:%d\n",count);
	char** args_array = (char**)malloc(sizeof(char*)*(args_count+2));
	args_array[0] = strdup(cmd);
	//(char*)malloc(sizeof(char)*(int)strlen(cmd)+1);
	//args_array[0] = cmd;
	//printf("%s\n",cmd);
	for (int i=1;i<=args_count+1;i++){
		if (i != args_count+1){
			//args_array[i] = (char*)malloc(sizeof(char)*(int)strlen(argv[i+count-args_count])+1);
			args_array[i] = strdup(argv[i+count-args_count-1]);
			printf("%s:%s\n",cmd,args_array[i]);
		}else {
			//args_array[i] = (char*)malloc(sizeof(char));
			args_array[i] = NULL ;
		}
	}
	int child_count = 1;
	int n_count = n;
	while(n_count--){
		
		//printf("%d:c:%d\n",n_count,child_count);
		fflush(stdout);
		pid_t child = fork();		
		if (child == -1) {
			print_fork_failed();
			return EXIT_FAILURE;
		}
		if (child){
			//parents
			int status;
			waitpid(child , &status ,0);
			child_count++;
			if (status)return 1;
		}else{
			//child
			for (int i=0; i<numpairs; i++){
				//printf("numtoken:%d i:%d\n",pairs[i].numtokens,numpairs);
				if (pairs[i].numtokens == 1){
				//	puts("1");
					//printf("temp:%c\n", pairs[i].values[1][0]);
					if(pairs[i].values[1][0] == '%'){
						char* temp = pairs[i].values[1]+1;
						//printf("temp:%s\n",temp);
						char* input_string = "";
						if (getenv(temp)!= NULL) input_string = getenv(temp);
						if (setenv(pairs[i].key,input_string,1)==-1)
							print_environment_change_failed();
					}else{
						if(setenv(pairs[i].key,pairs[i].values[1],1)==-1)
							print_environment_change_failed();
					}
				}else{
				//	puts("2");
				//	printf("%s:%s",pairs[i].key,pairs[i].values[child_count]);
					if(pairs[i].values[child_count][0] == '%'){
						char* temp = pairs[i].values[child_count]+1;
						char* input_string = "";
						if (getenv(temp)!= NULL) input_string = getenv(temp);
						//printf("temp:%s\n",temp);
						if(setenv(pairs[i].key,input_string,1)==-1)
							print_environment_change_failed();
					}else{
						if(setenv(pairs[i].key,pairs[i].values[child_count],1)==-1)
							print_environment_change_failed();
					}
					//if (setenv(pairs[i].key , pairs[i].values[child_count],1)==-1)
					//	print_environment_change_failed();
				}
			}
			execvp(cmd , args_array);
			print_exec_failed();
		}
	}
//	for (size_t i = 0; i < numtokens; i++) free(tokens[i]);
//	free(tokens);
	for (int i=0;i <= args_count; i++)free(args_array[i]);
	free(args_array);
	for (int i=0;i < numpairs; i++){
		for (int j = 0; j <= pairs[i].numtokens; j++){
			free(pairs[i].values[j]);
		}
		free(pairs[i].values);
	}
	free(pairs);
	//setenv("TZ", "EST5EDT",1);
	//print_environment_change_failed();
    return 0;
}

char **strsplit(const char* str, const char* delim, size_t* numtokens) {
    char *s = strdup(str);
    size_t tokens_alloc = 1;
    size_t tokens_used = 0;
    char **tokens = calloc(tokens_alloc, sizeof(char*));
    char *token, *rest = s;
    while ((token = strsep(&rest, delim)) != NULL) {
        if (tokens_used == tokens_alloc) {
            tokens_alloc *= 2;
            tokens = realloc(tokens, tokens_alloc * sizeof(char*));
        }
        tokens[tokens_used++] = strdup(token);
    }
    if (tokens_used == 0) {
        free(tokens);
        tokens = NULL;
    } else {
        tokens = realloc(tokens, tokens_used * sizeof(char*));
    }
    *numtokens = tokens_used;
    free(s);
    return tokens;
}

