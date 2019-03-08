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
int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck!
	graph *dependency_graph = parser_parse_makefile(makefile, targets);
	vector *target_vector = graph_vertices(dependency_graph);
	rule_t **rule = (rule_t**)malloc(sizeof(rule_t*) * vector_size(target_vector));
	vector **dependencies = (vector**)malloc(sizeof(vector*) * vector_size(target_vector));
	for (size_t i=0; i<vector_size(target_vector) ; i++){
		rule[i] = (rule_t *)graph_get_vertex_value(dependency_graph, (char*)vector_get(target_vector,i));
		dependencies[i] = graph_neighbors(dependency_graph, vector_get(target_vector,i));	
		printf("%s:",rule[i]->target);
		for (size_t j=0; j<vector_size(dependencies[i]) ; j++){
			printf("%s ",(char*)vector_get(dependencies[i],j));
		}
		printf("\n");
	}	
	queue *waiting_queue = queue_create(-1); 
	set *visited_set = int_set_create();
	int *data = malloc(sizeof(int)*vector_size(target_vector));
	for (size_t i=0; i<vector_size(target_vector) ; i++){
		data[i] = i;
	}
	int end = 0;
	/*size_t index = 0;
	while(end){
		if (vector_size(dependencies[index]) != 0){
			for (size_t i=0; i<vector_size(dependencies[index]) ; i++){
				set_add(waiting_set,vector_get(dependencies[index]),i);
			}	
			for (size_t j=0; j<vector_size(target_vector) ; j++){
				if(strcmp((char*)vector_get(dependencies[index],i),rule[j]->target) == 0 ){
					index = j;
					break;
				}
			}
		}else{
			
		}
	}*/
	while(vector_size(dependencies[0])!= 0 || end){
		
		for (size_t i=1; i<vector_size(target_vector) ; i++){
			//printf("test\n");
			bool cycle = true;
			size_t index = 0;
			for (index=0; index<vector_size(target_vector) ; index++){
				if (set_contains(visited_set,&data[index])) continue;
				if (vector_size(dependencies[index]) == 0){
					cycle = false;
					break;
				}
			}
			if (cycle) {
				end = 1;
				break;
			}
			set_add(visited_set,&data[index]);
			queue_push(waiting_queue,&data[index]);
			/*
			if (rule[index]->state != 2){
				rule[index]->state = 1; //success
				// execute command
				for (size_t j=0; j<vector_size(rule[index]->commands) ; j++){
					//printf("test\n");
					if (system(vector_get(rule[index]->commands,j)) != 0){
						rule[index]->state = 2; //fail
						break;
					}
				}
			}*/
			//printf("%s\n",rule[index]->target);
				// success execute command
			
			for (size_t j=0; j<vector_size(target_vector) ; j++){
				for (size_t k=0; k<vector_size(dependencies[j]) ; k++){
					if(strcmp((char*)vector_get(dependencies[j],k),rule[index]->target) == 0 ){
						//printf("erase:%s\n",(char*)vector_get(dependencies[j],k));
						if (rule[index]->state == 2)
							rule[j]->state = 2;
						vector_erase(dependencies[j],k);
					}
				}
			}
		}
	}
	if (vector_size(dependencies[0])== 0){
		printf("full\n");
	}
	//rule_t *rule = (rule_t *)graph_get_vertex_value(dependency_graph, target);
	graph_destroy(dependency_graph);
	vector_destroy(target_vector);
	free(rule);
	free(dependencies);
	free(data);
    return 0;
}
