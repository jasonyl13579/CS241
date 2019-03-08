/**
* Teaching Threads Lab
* CS 241 - Spring 2018
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reduce.h"
#include "reducers.h"

/* You might need a struct for each task ... */
struct task{
	pthread_t id;
	size_t start;
	size_t size;
	int *list;
	int index;
};
typedef struct task task;
static int* list_result;
static reducer reduce_f;
static int base_c;
/* You should create a start routine for your threads. */
void* my_func(void* task_input){
	task t= *((task*)task_input) ;
	list_result[t.index] = reduce(t.list + t.start, t.size, reduce_f, base_c);
    return NULL;
}
int par_reduce(int *list, size_t list_len, reducer reduce_func, int base_case,
               size_t num_threads) {
    /* Your implementation goes here */
  //  pthread_t tid; 
	if (num_threads > list_len) num_threads = list_len;
	task *t = calloc(num_threads , sizeof(task));
	size_t start = 0;
	int result = base_case;
	base_c = base_case;
	reduce_f = reduce_func;
	list_result = calloc (num_threads , sizeof(int));
//	for (size_t j=0;j<list_len;j++) printf(",%c",list[j]);
	
	for(size_t i =0; i < num_threads; i++) {
		t[i].index = i;
		t[i].list = list;
        t[i].start = start;
	//	printf("%zd\n",start);	
		if (i != num_threads-1)
			t[i].size = list_len/num_threads;
		else 
			t[i].size = list_len - start;
	//	printf("s:%zd\n",t[i].size);
		start += list_len/num_threads;
    }
    for(int i =0; i < (int)num_threads; i++) {
        pthread_create(&t[i].id, NULL, my_func, &t[i]); 
    }
	void* r;
	for(int i =0; i < (int)num_threads; i++) {
        pthread_join(t[i].id, &r);
    }
 
	for (size_t i =0; i < num_threads; i++){
		result = reduce_func(result, list_result[i]);
	}
	free(list_result);
	free(t);
	//sprintf("result:%d\n",result);
	return result;
}
