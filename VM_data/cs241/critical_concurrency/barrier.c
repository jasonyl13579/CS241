/**
* Critical Concurrency Lab
* CS 241 - Spring 2018
*/

#include "barrier.h"
#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h>
// The returns are just for errors if you want to check for them.
int barrier_destroy(barrier_t *barrier) {
    int error = 0;
	pthread_mutex_destroy(&barrier->mtx);
	pthread_cond_destroy(&barrier->cv);
	//free(barrier);
    return error;
}


int barrier_init(barrier_t *barrier, unsigned int num_threads) {
    int error = 0;
	//barrier = (barrier_t)malloc(sizeof(barrier_t));
	barrier->count = 0;
	barrier->n_threads = num_threads;
	//printf("%d %d\n",barrier->count,barrier->n_threads);
	pthread_mutex_init(&barrier->mtx, NULL);
	pthread_cond_init(&barrier->cv, NULL);
	barrier->times_used = 0;
    return error;
}

int barrier_wait(barrier_t *barrier) {
	pthread_mutex_lock(&barrier->mtx);
	//usleep(100);
	if (barrier->count == barrier->n_threads ){
		//puts("test");
		while(barrier->times_used != 0){
			pthread_cond_wait(&barrier->cv, &barrier->mtx); 
		}
	}
	if (barrier->count == barrier->n_threads) barrier->count=0;
	barrier->count++; 
	//printf("%d %d %d\n",barrier->count,barrier->n_threads,barrier->times_used);
	if (barrier->count == barrier->n_threads ) { 
		barrier->times_used+=barrier->n_threads-1;
		pthread_cond_broadcast(&barrier->cv); 
		//puts("test");
		//fflush(stdout);
		//sleep(1);
	}
	else {
		while(barrier->count != barrier->n_threads ) { 
			pthread_cond_wait(&barrier->cv, &barrier->mtx); 
		}
		barrier->times_used --;
		//printf("t:%d\n",barrier->times_used);
		if (barrier->times_used == 0)pthread_cond_broadcast(&barrier->cv); 
	}
	pthread_mutex_unlock(&barrier->mtx);
    return 0;
}
