/**
* Pointers Gone Wild Lab
* CS 241 - Spring 2018
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "callbacks.h"
#include "part3-functions.h"
#include "vector.h"

vector *vector_map(vector *input, mapper map) {
    // TODO implement this
	char* temp;
	vector *output_vector = string_vector_create();
	VECTOR_FOR_EACH(input, elem, {
        if (_it != _iend){
			temp = map(elem); 
            vector_push_back(output_vector,temp);
			free(temp);
			//free(elem);
		}
    });
    return output_vector;
}

void *vector_reduce(vector *input, reducer reduce, void *acc) {
    // TODO implement this
	//void* acc2= (void*)malloc(sizeof(int));
	VECTOR_FOR_EACH(input, elem, {
        if (_it != _iend){
           acc = reduce(elem,acc);
		//   free(elem);
		}
    });
	//free(acc);
    return acc;
}

void *length_reducer(char *input, void *output) {
    // TODO implement this
	//static int x = 0;
	int *o ;
	int size = 0;
	while (*input++) size++;
	if (output == NULL){
		output = (void*)malloc(sizeof(int));
		o = output;
		*o = 0;
	}
	o = output;
	*o = *(int*)output + size;
	//printf("%d:%d\n",size,*o);
	//*n = *n+*(int*)output;
    return output;
}

void *concat_reducer(char *input, void *output) {
    // TODO implement this
	char*t = input ;
	char*o ;
	int size = 0;
	while (*t++) size++;
	//printf("%s:%d\n",input,size);
	if(output == NULL){
	//	puts("ttttt");
		output = (void*)calloc(size+1,sizeof(char*));	
		o = output;
		while(*input){
			 *o = *input;
			 o++; input++;
		}
	}else{ 
		o = (char*)output;
		while(*o){
		//	printf("0:%c\n",*o);
			size++;o++;		
		}
		output = (void*)realloc(output,size*sizeof(char*)+1);
		o = output;
		while(*o){
		//	printf("0:%c\n",*o);
			o++;		
		}
		//printf("%d\n",size);
		while(*input){
			 *o = *input;
			 //printf("c:%c\n",*input);
			 o++; input++;
		}
	}
	
    return output;
}
