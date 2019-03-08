/**
* Mini Valgrind Lab
* CS 241 - Spring 2018
*/

#include "mini_valgrind.h"
#include <stdio.h>
#include <string.h>
meta_data *head ;
size_t total_memory_requested;
size_t total_memory_freed;
size_t invalid_addresses;

void *mini_malloc(size_t request_size, const char *filename,
                  void *instruction) {
    // your code here
	if (request_size == 0) return NULL;
	meta_data *cur = head;
	meta_data *temp = NULL;
	while (cur){
		temp = cur;
		cur = cur->next;
	}
	cur = malloc(sizeof(meta_data)+request_size);
	if (cur == NULL) return NULL;
	cur->filename = filename;
	cur->instruction = instruction;
	cur->request_size = request_size;
	
	if (head == NULL){ head = cur; head->next = NULL;}
	else temp->next = cur;
	total_memory_requested += request_size;
//	fprintf(stderr,"malloc:%p,%p,size:%zd\n",cur + 1,cur,request_size);
    return cur + 1;
}

void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {
    // your code here
	if (num_elements == 0 || element_size == 0) return NULL;
	void* temp = mini_malloc( num_elements*element_size, filename, instruction);
	if (temp == NULL) return NULL;
	memset(temp, 0, num_elements*element_size);
//	fprintf(stderr,"calloc:%p\n",temp);
    return temp;
}

void *mini_realloc(void *payload, size_t request_size, const char *filename,
                   void *instruction) {
    // your code here
	//fprintf(stderr,"realloc:payload %p,size:%zd\n",payload,request_size);
	if (payload == NULL){
		void *ptr = mini_malloc(request_size, filename, instruction);
		return ptr;
	}
	if (request_size == 0){
		mini_free(payload);
		return NULL;
	}
	//fprintf(stderr,"realloc:payload %p,size:%zd\n",payload,request_size);
	//fprintf(stderr,"invalid:%zd\n",invalid_addresses);
	void *tmp = malloc(sizeof(meta_data)+request_size);
	meta_data *meta_data_p = payload - sizeof(meta_data);
//	fprintf(stderr,"realloc:meta_data_p:%p\n",meta_data_p);
	size_t original_size = meta_data_p->request_size;
	memcpy(tmp, payload, request_size < original_size ? request_size : original_size);
	
	size_t invalid_address = invalid_addresses;
	mini_free(payload);
	if (invalid_addresses != invalid_address){
		free(tmp);
		return NULL;
	}
	void *ptr = mini_malloc(request_size, filename, instruction);
//	meta_data *meta_data_p = payload - sizeof(meta_data);
//	fprintf(stderr,"realloc:meta_data_p:%p\n",meta_data_p);
//	size_t original_size = meta_data_p->request_size;
	memcpy(ptr, tmp, request_size < original_size ? request_size : original_size);
	
	if (request_size >= original_size) {
		total_memory_requested -= original_size;
		total_memory_freed -= original_size;
	}else{
		total_memory_requested -= request_size;
		total_memory_freed -= request_size;
	}
//	fprintf(stderr,"invalid:%zd\n",invalid_addresses);
	free(tmp);
    return ptr;
	
}

void mini_free(void *payload) {
    // your code here
	//if (payload !=NULL)
	//	fprintf(stderr,"payload %p\n",payload);
	if (payload){
		meta_data *meta_data_p = payload - sizeof(meta_data);
//		fprintf(stderr,"m_p %p\n",meta_data_p);
		meta_data *cur = head;
//		fprintf(stderr,"head %p\n",cur);
		meta_data *temp = NULL;
		if (meta_data_p == head){
		//	fprintf(stderr,"head n%p\n",head->next);
			if (head->next == NULL) head = NULL;
			else head = head->next;
			cur->next = NULL;
			total_memory_freed += cur->request_size;
		//	fprintf(stderr,"free1 %p\n",cur);
			free(cur);
			return;
		}
		while (cur){
	//		fprintf(stderr,"cur %p\n",cur);
			if( cur == meta_data_p){
				temp->next = cur->next;
				cur->next = NULL;
				total_memory_freed += cur->request_size;
		//		fprintf(stderr,"free2 %p\n",cur);
				free(cur);
				return;
			}else{
				temp = cur; 
				cur = cur->next;
			}
		}
	}else return;
	//fprintf(stderr,"invalid_free %p\n",payload);
	invalid_addresses++;
	return;
}
