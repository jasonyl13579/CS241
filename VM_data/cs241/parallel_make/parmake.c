/**
* Parallel Make Lab
* CS 241 - Spring 2018
*/


#include "format.h"
#include "graph.h"
#include "parmake.h"
#include "parser.h"
#include "stdio.h"
#include "stdlib.h"
#include "set.h"
#include "queue.h"
#include <time.h>
#include <sys/stat.h>
#include <pthread.h>
#define FAIL 2
void search_dependencies(rule_t **, vector **, set *, vector *, size_t index, bool child);
static bool cycle = false;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t active_thread_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv;
static pthread_cond_t main_cv;
static int queue_size = 0;
static queue *waiting_queue ;
static rule_t **rule ;
static int active_thread = 0;
static vector *target_vector;
static vector **dependencies;
static int end ;
static time_t getFileModifiedTime(const char *path)
{
    struct stat attr;
    if (stat(path, &attr) == 0)
    {
       // printf("%s: last modified time: %s", path, ctime(&attr.st_mtime));
        return attr.st_mtime;
    }
    return 0;
}
void* command_executer(void* input){
	//int index = *(int*) input;
	//printf("%d\n",index);
	while (1){		
		pthread_mutex_lock(&mutex);		
		while (queue_size == 0 && !end){
			pthread_cond_wait(&cv, &mutex);
		}
		if (end) {
			//printf("thread_end\n");	
			pthread_mutex_unlock(&mutex);
			break;
		}
		int index = *(int*)queue_pull(waiting_queue);
		queue_size--;
		pthread_mutex_unlock(&mutex);
		/*pthread_mutex_lock(&active_thread_mutex);
		active_thread ++ ;
		printf("add_thread:%d\n",active_thread);
		pthread_mutex_unlock(&active_thread_mutex);*/
		if (rule[index]->state != FAIL && rule[index]->state != 1){
			rule[index]->state = 1; //success
					// execute command
			for (size_t j=0; j<vector_size(rule[index]->commands) ; j++){
				//printf("test\n");
				if (system(vector_get(rule[index]->commands,j)) != 0){
					rule[index]->state = FAIL; //fail
					break;
				}
			}
		}
		pthread_mutex_lock(&mutex);	
		for (size_t j=0; j<vector_size(target_vector) ; j++){
			for (size_t k=0; k<vector_size(dependencies[j]) ; k++){
				if(strcmp((char*)vector_get(dependencies[j],k),rule[index]->target) == 0 ){
					//printf("erase:%s\n",(char*)vector_get(dependencies[j],k));
					if (rule[index]->state == FAIL)
						rule[j]->state = FAIL;					
					vector_erase(dependencies[j],k);
				}
			}
		}
		pthread_mutex_unlock(&mutex);
		pthread_mutex_lock(&active_thread_mutex);
		active_thread -- ;
		pthread_cond_signal(&main_cv);
		pthread_mutex_unlock(&active_thread_mutex);
	}	
	return NULL;
}
int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck!
	graph *dependency_graph = parser_parse_makefile(makefile, targets);
	target_vector = graph_vertices(dependency_graph);
	rule = (rule_t**)malloc(sizeof(rule_t*) * vector_size(target_vector));
	dependencies = (vector**)malloc(sizeof(vector*) * vector_size(target_vector));
	for (size_t i=0; i<vector_size(target_vector) ; i++){
		rule[i] = (rule_t *)graph_get_vertex_value(dependency_graph, (char*)vector_get(target_vector,i));
		dependencies[i] = graph_neighbors(dependency_graph, vector_get(target_vector,i));	
		//printf("%s:",rule[i]->target);
		for (size_t j=0; j<vector_size(dependencies[i]) ; j++){
			//printf("%s ",(char*)vector_get(dependencies[i],j));
		}
		//printf("\n");
	}	
	pthread_cond_init(&cv, NULL);
	pthread_cond_init(&main_cv, NULL);
	waiting_queue = queue_create(-1); 	
	set *visited_set = int_set_create();
	set *waiting_set = string_set_create();
	int *data = malloc(sizeof(int)*vector_size(target_vector));
	for (size_t i=0; i<vector_size(target_vector) ; i++){
		data[i] = i;
	}
	end = 0;
	//thread
	pthread_t *t = malloc(sizeof(pthread_t)*num_threads);
	for(int i =0; i < (int)num_threads; i++) {
		pthread_create(&t[i], NULL, command_executer, &data[i]);
	}
	//thread
	
	
	size_t index = 0;
	search_dependencies(rule,dependencies,waiting_set,target_vector,index, false);
	//vector *test = set_elements(waiting_set);
	//for (size_t i=0; i<vector_size(test) ; i++){
		//printf("t:%s\n",(char*)vector_get(test,i));
	//}
	for (size_t count=0; count<vector_size(target_vector) ; count++){	
		if (!set_contains(waiting_set,rule[count]->target))
			set_add(visited_set,&data[count]);
	}
	while(vector_size(dependencies[0])!= 0 && !end){
		//puts("start");
		for (size_t count=0; count<vector_size(target_vector) ; count++){			
			size_t index = 0;
			for (index=0; index<vector_size(target_vector) ; index++){
				if (set_contains(visited_set,&data[index])) continue;
				if (vector_size(dependencies[index]) == 0){
					break;
				}
			}	
			//printf("chosen_index:%zd\n",index);
			if (index >= vector_size(target_vector) ) {	
				pthread_mutex_lock(&active_thread_mutex);
				if (active_thread != 0){
					//puts("test");
					while (active_thread != 0){
						for (index=0; index<vector_size(target_vector) ; index++){
							if (set_contains(visited_set,&data[index])) continue;
							if (vector_size(dependencies[index]) == 0){
								break;
							}
						}
						if (index < vector_size(target_vector)) break;
						pthread_cond_wait(&main_cv, &active_thread_mutex);
					}
					//puts("test2");
					pthread_mutex_unlock(&active_thread_mutex);
					if (vector_size(dependencies[0]) == 0) {
						pthread_mutex_lock(&mutex);
						end = 1;
						pthread_cond_broadcast(&cv);
						pthread_mutex_unlock(&mutex);
						//printf("S:%d\n",queue_size);
					}
					break;
				}else{
					pthread_mutex_unlock(&active_thread_mutex);
					vector *visited_vector = set_elements(visited_set);
					if (vector_size(target_vector) != vector_size(visited_vector)) cycle = true;
					pthread_mutex_lock(&mutex);
					end = 1;
					pthread_cond_broadcast(&cv);
					pthread_mutex_unlock(&mutex);
					//puts("end");
					break;
				}
			}
			set_add(visited_set,&data[index]);
			queue_push(waiting_queue,&data[index]);		
			pthread_mutex_lock(&mutex);
			queue_size++;
			//printf("queue_size:%d\n",queue_size);
			pthread_cond_broadcast(&cv);
			pthread_mutex_unlock(&mutex);
			pthread_mutex_lock(&active_thread_mutex);
			active_thread++;
			//printf("add_thread:%d\n",active_thread);
			pthread_mutex_unlock(&active_thread_mutex);
			//printf("index:%zd\n",index);
			
			//printf("%s\n",rule[index]->target);
				// success execute command
			

		}
	}
	//end = 0;
	void* result; 
	for(int i =0; i < (int)num_threads; i++) {
		pthread_join(t[i], &result);
	}
	if(cycle){
		for (size_t i=0; i<vector_size(dependencies[0]) ; i++){
			print_cycle_failure(vector_get(dependencies[0],i));
		}
	}
	
	for (size_t j=0; j<vector_size(target_vector) ; j++){
		vector_destroy(dependencies[j]);
	}
	set_destroy(waiting_set);
	set_destroy(visited_set);
	graph_destroy(dependency_graph);
	vector_destroy(target_vector);
	free(rule);
	free(dependencies);
	free(data);
    return 0;
}
void search_dependencies(rule_t **rule,vector **dependencies, set *waiting_set,vector *target_vector, size_t index, bool child){
	//printf("index:%s\n",rule[index]->target);
	if (vector_size(dependencies[index]) != 0){
		
		bool file = false;
		if (getFileModifiedTime(rule[index]->target) !=0){
			file = true;
			for (size_t i=0; i<vector_size(dependencies[index]) ; i++){
				if (getFileModifiedTime(vector_get(dependencies[index],i))==0 ){
					file = false;
				}
			}
		}
		/*if (set_contains(waiting_set,rule[index]->target) && !child){
			//printf("dup:%s\n",rule[index]->target);
			return;
		}*/
		for (size_t i=0; i<vector_size(dependencies[index]) ; i++){
			//printf("index:%s\n",(char*)vector_get(dependencies[index],i));
			if (file){
				time_t new = getFileModifiedTime(rule[index]->target);
				time_t old = getFileModifiedTime(vector_get(dependencies[index],i));
				if (new - old > 1) {
					vector_erase(dependencies[index],i);
					rule[index]->state = 1;
					break;
				}
			}
			if (set_contains(waiting_set,vector_get(dependencies[index],i))){
				//cycle = true;
				continue;
				/*if (index == 0)
					print_cycle_failure(vector_get(dependencies[index],i));
				if (i != vector_size(dependencies[index])-1 )
					continue;
				else
					return;*/
			}
			else{
				set_add(waiting_set,vector_get(dependencies[index],i));
			}
			for (size_t j=0; j<vector_size(target_vector) ; j++){
				if(strcmp((char*)vector_get(dependencies[index],i),rule[j]->target) == 0 ){
					search_dependencies(rule,dependencies,waiting_set,target_vector,j,true);	
					//printf("j:%s\n",rule[j]->target);
				}
			}				
		}
	}	
}
