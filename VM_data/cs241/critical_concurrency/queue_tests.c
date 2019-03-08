/**
* Critical Concurrency Lab
* CS 241 - Spring 2018
*/

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "queue.h"
#define NUM_THREADS 100
 static queue *q;
 //static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_t pthreads[NUM_THREADS];
static void *thread_func_queue(void *var) {
//	pthread_mutex_lock(&mtx);
//	for (int i=0 ; i<100;i++)
//    queue_pull(q);
 //  sleep(1);
	queue_push(q,var);
//	pthread_mutex_unlock(&mtx);
	sleep(1);
//	pthread_mutex_lock(&mtx);
	
//	int i = *(int*)var;
//	i = i * 10;
//	sleep(1);
//	queue_push(q,var);
	
	queue_pull(q);
//	pthread_mutex_unlock(&mtx);
    return 0;
}
/*static void *thread_pull_queue(void *var) {
//for (int i=0 ; i<10;i++)
//	queue_pull(q);
//	sleep(1)
//	queue_pull(q);
	
//	pthread_mutex_unlock(&mtx);
    return 0;
}*/
int main(int argc, char **argv) {
    /*if (argc != 2) {
        printf("usage: %s test_number\n", argv[0]);
        exit(1);
    }*/
    printf("Please write tests cases\n");
	void* ret;
    q = queue_create(-1);
	int data[NUM_THREADS];
	for (int i = 0; i < NUM_THREADS; i++){
		data[i] = i;
	}
	/*queue_push(q, (void*)&data[1]);
	queue_push(q, (void*)&data[1]);
	queue_push(q, (void*)&data[1]);
	queue_pull(q);
	queue_push(q, (void*)&data[1]);*/
	
	for (long int i = 0; i < NUM_THREADS; i++) {
	//	printf("%ld start\n",i);
        pthread_create(&pthreads[i], NULL, &thread_func_queue, (void*)&data[i]);
		//usleep(500);
		//queue_push(q, (void*)&data[i]);
    }
	
	//for (long int i = NUM_THREADS/2; i < NUM_THREADS; i++) {
	//	printf("pull:%ld start\n",i);
     //   pthread_create(&pthreads[i], NULL, &thread_pull_queue, (void*)&data[i]);
		//usleep(500);
		//queue_pull(q);
   // }
	for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(pthreads[i], (void **)(&ret));
        //printf("%ld\n", ret);
        //if (ret != 5)
         //   pass1 = 0;
    }
	queue_destroy(q);
    return 0;
}
