/**
* Password Cracker Lab
* CS 241 - Spring 2018
*/

#include "cracker1.h"
#include "format.h"
#include "utils.h"
#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "queue.h"
#include <crypt.h>
#include "math.h"
struct task{
	pthread_t id;
	int threadId;
	char* name;
	char* hash;
	char* password;
};
typedef struct task task;
static queue *queue_task;
static int numRecovered = 0; 
static int numFailed = 0;
static int password_remain;
static size_t password_count = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
char **strsplit(const char *,const char *, size_t *);
void free_args(char **);
void* password_cracker(void* task_input){
	while(1){
		double startTime = getThreadCPUTime();
		int index = *((int*)task_input) ;
		pthread_mutex_lock(&mtx);
		if ( password_remain == 0) {
			pthread_mutex_unlock(&mtx);
			break;
		}
		task *t = (task*)queue_pull(queue_task);
		password_remain --;
		pthread_mutex_unlock(&mtx);
		//printf("%p\n",t);
		v1_print_thread_start(index,t->name);
		struct crypt_data cdata;
		cdata.initialized = 0;
		int password_length = strlen(t->password);		
		char* answer = t->password;
		int start = getPrefixLength(t->password);
		char* temp = answer + start;
		for (int i = start ; i < password_length ; i++)
			answer[i] = 'a';
		int result = 1;
		char *hashed;
		int hashCount = 1;
		while(result){
			hashed = crypt_r(answer, "xx", &cdata);
			if (strcmp(hashed, t->hash)==0) break;
			//printf("%s\n%s\n",answer,hashed);
			result = incrementString(temp);
			if (result) hashCount ++;
		}
		double timeElapsed = getThreadCPUTime() - startTime;
		//printf("hash of '%s' = %s\n", answer,t->hash);
		pthread_mutex_lock(&mtx);
		if (result){
			numRecovered++;
		}else numFailed ++;
		pthread_mutex_unlock(&mtx);
		v1_print_thread_result(index, t->name, answer, hashCount, timeElapsed, !result);
	}
	//printf("test\n");
    return NULL;
}

int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads;
	
	char *buffer = NULL;
	size_t size = 0;
	ssize_t chars ;
	
	pthread_t *t = malloc(sizeof(pthread_t)*thread_count);
	task *password_t = malloc(sizeof(task));
	queue_task = queue_create(-1);
	
	while((chars = getline(&buffer, &size, stdin))!= -1){
				// Discard newline character if it is present,			
		if (chars > 0 && buffer[chars-1] == '\n') 
			buffer[chars-1] = '\0';
		//printf("%s\n",buffer);
		char **tokens;
		size_t numtokens;
		password_count++ ;
		password_t = realloc(password_t,sizeof(task)*password_count);
		tokens = strsplit(buffer, " ", &numtokens);	
		password_t[password_count-1].name = strdup(tokens[0]);
		password_t[password_count-1].threadId = 0;
		//printf("p:%p\n",(void*)&password_t[password_count-1]);
		
		password_t[password_count-1].hash = strdup(tokens[1]);
		password_t[password_count-1].password = strdup(tokens[2]);
		//usleep(2000);
			
		free_args(tokens);		
	}
	 password_remain = password_count;
	for (size_t i=0; i<password_count ; i++){
		queue_push(queue_task,(void*)&password_t[i]);
		//printf("%p\n",&password_t[i]);
	}
	//printf("%zd\n",password_count);
	//usleep(1000);
	
	int data[thread_count];
	for (size_t i = 1; i <= thread_count; i++){
		data[i-1] = i;
	}
//	if (password_count >= thread_count){
		for(int i =0; i < (int)thread_count; i++) {
			//task *temp = (task*)queue_pull(queue_task);
			//printf("p:%p\n",temp);
			//printf("%s\n",temp->name);
			//temp->threadId = i+1;
			pthread_create(&t[i], NULL, password_cracker, &data[i]);
		}
		void* result; 
		for(int i =0; i < (int)thread_count; i++) {
			pthread_join(t[i], &result);
		}
//	}
	v1_print_summary(numRecovered,numFailed);
	free(buffer);
	for (size_t i=0; i<password_count ; i++){
		free(password_t[i].name);
		free(password_t[i].hash);
		free(password_t[i].password);
	}
	free(password_t);
	queue_destroy(queue_task);
	free(t);
    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}


char **strsplit(const char *str, const char *delim, size_t *numtokens) {
    // copy the original string so that we don't overwrite parts of it
    // (don't do this if you don't need to keep the old line,
    // as this is less efficient)
    char *s = strdup(str);
    // these three variables are part of a very common idiom to
    // implement a dynamically-growing array
    size_t tokens_alloc = 1;
    size_t tokens_used = 0;
    char **tokens = calloc(tokens_alloc, sizeof(char *));
    char *token, *strtok_ctx;
    for (token = strtok_r(s, delim, &strtok_ctx); token != NULL;
         token = strtok_r(NULL, delim, &strtok_ctx)) {
        // check if we need to allocate more space for tokens
        if (tokens_used == tokens_alloc) {
            tokens_alloc *= 2;
            tokens = realloc(tokens, tokens_alloc * sizeof(char *));
        }
        tokens[tokens_used++] = strdup(token);
    }
    // cleanup
    if (tokens_used == 0) {
        free(tokens);
        tokens = NULL;
    } else {
        tokens = realloc(tokens, tokens_used * sizeof(char *));
    }
    *numtokens = tokens_used;
    free(s);
    // Adding a null terminator
    tokens = realloc(tokens, sizeof(char *) * (tokens_used + 1));
    tokens[tokens_used] = NULL;
    return tokens;
}

void free_args(char **args) {
    char **ptr = args;
    while (*ptr) {
        free(*ptr);
        ptr++;
    }
    free(args);
}