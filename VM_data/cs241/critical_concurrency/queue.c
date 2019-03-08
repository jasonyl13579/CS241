/**
* Critical Concurrency Lab
* CS 241 - Spring 2018
*/

#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * This queue is implemented with a linked list of queue_nodes.
 */
typedef struct queue_node {
    void *data;
    struct queue_node *next;
} queue_node;

struct queue {
    /* queue_node pointers to the head and tail of the queue */
    queue_node *head, *tail;

    /* The number of elements in the queue */
    ssize_t size;

    /**
     * The maximum number of elements the queue can hold.
     * max_size is non-positive if the queue does not have a max size.
     */
    ssize_t max_size;

    /* Mutex and Condition Variable for thread-safety */
    pthread_cond_t cv;
    pthread_mutex_t m;
};

queue *queue_create(ssize_t max_size) {
    /* Your code here */
	queue *q = malloc(sizeof(queue));
	if (max_size < 0) max_size = -1;
	q->max_size = max_size;
	q->size = 0;
	q->head = 0;
	q->tail = 0;
	pthread_mutex_init(&q->m, NULL);
	pthread_cond_init(&q->cv, NULL);
	//q.head = malloc (sizeof(queue_node));	
    return q;
}

void queue_destroy(queue *this) {
    /* Your code here */
	queue_node *cur = this->head;
	queue_node *temp = 0;
	while(cur){
		temp = cur;
		cur = cur -> next;
		free(temp);
	}
	pthread_mutex_destroy(&this->m);
	pthread_cond_destroy(&this->cv);
	free(this);
}

void queue_push(queue *this, void *data) {
    /* Your code here */
	pthread_mutex_lock(&this->m);
	//puts("1");
	//printf("push_in:%d,size:%zd\n", (int)data,this->size);
	while(this->size == this->max_size && this->max_size > 0){
		
		pthread_cond_wait(&this->cv, &this->m);		
	}
	
	queue_node *node = malloc (sizeof(queue_node));
	node->next = 0;
	node->data = data;
	if (this->head == 0){	
		this->head = node;
	}else{
		queue_node *cur = this->head;
		queue_node *temp = NULL;
		while(cur) {
			temp = cur;
			cur = cur->next;
		}		
		temp->next = node;
		cur = node;		 
	}
//	this->tail = node;
	this->size ++;
	//fprintf(stderr,"push:%d,size:%zd\n", *(int*)data,this->size);
	if (this->size == 1) pthread_cond_broadcast(&this->cv);
	pthread_mutex_unlock(&this->m);
}

void *queue_pull(queue *this) {
    /* Your code here */
	pthread_mutex_lock(&this->m);
	//printf("pull-size:%zd\n",this->size);
	//printf("pull_in:%d,size:%zd\n",*(int*)this->head->data,this->size);
	while(this->size == 0 ){
		//pthread_mutex_unlock(&this->m);
		// return NULL;
		pthread_cond_wait(&this->cv, &this->m);
	}		
	//fprintf(stderr,"pull:%d,size:%zd\n",*(int*)this->head->data,this->size-1);
	queue_node *cur = this->head;
	void * result = this->head->data;
	this->head = this->head->next;
	cur -> next = 0;
	free(cur);
	this->size --;
	
	if (this->size == this->max_size-1){
		pthread_cond_broadcast(&this->cv);
	}
	//fprintf(stderr,"result:%d,size:%zd\n",*(int*)result,this->size);
	pthread_mutex_unlock(&this->m);
    return result;
}
