/**
* Malloc Lab
* CS 241 - Spring 2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

typedef struct _meta_data {
    // Number of bytes of heap memory the user requested from malloc
    void *ptr;
	size_t size;
    struct _meta_data *next;
	struct _meta_data *previous;
} meta_data;
typedef struct _free_data {
    // Number of bytes of heap memory the user requested from malloc
    struct _free_data *next;
	struct _free_data *previous;
} free_data;
static meta_data *head = NULL;
static free_data *free_head = NULL; 
static size_t maxsize = 0;
static int reverse_search = 0;
/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
    // implement calloc!
	//fprintf(stderr,"calloc\n");
	void *p = malloc (num * size);
	if (p == NULL) return NULL;
	memset(p,0,num * size);
    return p;
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
 // lowest bit = 1 -> free
 void *malloc(size_t size) {
    // implement malloc!
	//fprintf(stderr,"malloc size:%zd\n",size);
	free_data *free_node = free_head;
	meta_data *chosen = NULL;
	size = (size + sizeof(meta_data) + 15)/16*16 - sizeof(meta_data);
	//fprintf(stderr,"malloc size:%zd\n",size);
	while(free_node!= NULL){
		if (size > maxsize) break;
	//	fprintf(stderr,"s:%zd\n",p->size);
		meta_data *p = (void*)free_node - sizeof(meta_data); 
		if ((p->size & 1)&& (p->size & ~1) >= size){  //encode free in size
			
			if (chosen == NULL || (chosen && p->size < chosen->size)){
				 chosen = p ;
				 break;
			}
		}
		if (reverse_search == 0)
			free_node = free_node->previous;
		else 
			free_node = free_node->next;
		//if (p == NULL ) fprintf(stderr,"weird thing@@@@@@@@@@@@@@@@@@\n");
		if (free_node == free_head) break;
	}
	reverse_search = 0;
	//fprintf(stderr,"malloc address:%p,max:%zd\n",chosen,maxsize);
	if (chosen){
		//check split
		//fprintf(stderr,"choose:%zd\n", chosen->size);
		chosen->size = chosen->size & ~1;
		
			if (chosen->size < size + sizeof(meta_data)+15) {
				free_node = (free_data*)(chosen + 1);
				free_node->previous->next = free_node->next;
				free_node->next->previous = free_node->previous;
				return chosen->ptr;
			}else{
				//split
					size_t original_size = chosen->size;
					chosen->size = size;
					meta_data *new = (meta_data *)(chosen->ptr + size);
					new->ptr = new + 1;
					new->size = original_size - size - sizeof(meta_data);
					new->size = new->size | 1;
					new->previous = chosen;
					new->next = chosen->next;
					memmove(new+1,chosen+1,sizeof(free_node));
					chosen->next->previous = new;
					chosen->next = new;
					//fprintf(stderr,"split-o_size:%zd,size:%zd,%zd\n",original_size,chosen->size,new->size-1);
				//split
			}
		//check split	
		//fprintf(stderr,"ptr:%p,start:%p,end:%p\n",chosen,chosen->ptr,chosen->ptr+chosen->size);
		return (void*)chosen->ptr;
	}	
	void* temp ;
	
	if ((temp = sbrk((long int)(sizeof(meta_data)+size))) == (void*)-1){
		//fprintf(stderr,"fail to allocate sbrk\n");
		return NULL;
	}
	
	chosen = (meta_data*)temp;
	chosen->ptr =(meta_data*) (temp + sizeof(meta_data));
	chosen->size = size & ~1;
	if (head == NULL){
		head = chosen ;
		chosen->next = chosen;
		chosen->previous = chosen;
	}else{
		meta_data *tail = head->previous;
		tail->next = chosen;
		chosen->next = head;
		chosen->previous = tail;
		head->previous = chosen;
	}
//	fprintf(stderr,"ptr:%p,start:%p,size:%zd~~~~~~~~~~~~~~~~~~~~~~\n",chosen,chosen->ptr,chosen->size);
	return (void*)chosen->ptr;
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
    // implement free!
	//fprintf(stderr,"free:%p\n",ptr);
	if (ptr == NULL) return;
	meta_data *meta_data_p = ptr - sizeof(meta_data);
	meta_data_p->size = meta_data_p->size | 1; // set lowest bit to 1
	//fprintf(stderr,"p:%p\n",meta_data_p);
	free_data *chosen = (free_data*)(meta_data_p + 1);
	//fprintf(stderr,"p:%p \n",chosen);
	if (free_head == NULL){
		free_head = chosen;
		free_head->next = free_head;
		free_head->previous = free_head;
	}
	int merge = 0;
	if (meta_data_p -> next != head && meta_data_p->next->size & 1){
		fprintf(stderr,"merge1\n");
		meta_data_p->size += (meta_data_p ->next->size + sizeof (meta_data)-1); //already set free to 1
		fprintf(stderr,"size:%zd,%p\n",meta_data_p->size,meta_data_p);
		meta_data *delete = meta_data_p ->next;
		meta_data_p ->next->next->previous = meta_data_p;	
		meta_data_p-> next = meta_data_p ->next->next;
		delete->next = NULL;
		delete->previous = NULL;	
		/*free_data *chosen = (free_data*)(delete + 1);
		fprintf(stderr,"p1:%p \n",chosen);
		chosen->previous->next = chosen->next;
		chosen->next->previous = chosen->previous; */
		chosen = (free_data*)(meta_data_p + 1);
		free_data *tail = free_head->previous;
		tail->next = chosen;
		chosen->next = free_head;
		chosen->previous = tail;
		free_head->previous = chosen;		
		merge = 1;
	}
	if (meta_data_p != head && meta_data_p->previous->size & 1){ 
		fprintf(stderr,"merge2\n");
		meta_data_p->previous->size += (meta_data_p->size + sizeof (meta_data)-1); //already set free to 1
		fprintf(stderr,"size:%zd,%p\n",meta_data_p->previous->size,meta_data_p->previous);
		
		meta_data_p->previous->next = meta_data_p->next;
		meta_data_p->next->previous = meta_data_p->previous;
		meta_data_p->next = NULL;
		meta_data_p->previous = NULL;
		
		chosen = (free_data*)(meta_data_p->previous + 1);
		free_data *tail = free_head->previous;
		tail->next = chosen;
		chosen->next = free_head;
		chosen->previous = tail;
		free_head->previous = chosen;
		merge = 1;		
	}
	if (!merge){
		free_data *tail = free_head->previous;
		tail->next = chosen;
		chosen->next = free_head;
		chosen->previous = tail;
		free_head->previous = chosen;
	}
	if (maxsize < meta_data_p->size) maxsize = meta_data_p->size;
	return;

}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
    // implement realloc!
	
	if (ptr == NULL) return malloc(size);
	if (size == 0){
		free (ptr);
		return NULL;
	}
	size = (size + sizeof(meta_data) + 15)/16*16 - sizeof(meta_data);
	//fprintf(stderr,"realloc-size:%zd,%p!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",size,ptr);
	meta_data *p = ptr - sizeof(meta_data);
	if ( size == p->size) return ptr;
	else if ( size > p->size){
		//fprintf(stderr,"bigger_r\n");
		if ((p->next!= head)&&(p->next->size & 1)&&size < p->size + p->next->size + sizeof(meta_data)){
					
			if (size >= p->size + p->next->size-1 ){	// no new node
				size_t sum = p->size + p->next->size + sizeof(meta_data);
				p->next->next->previous = p;
				p->next = p->next->next;
				p->size = sum & ~1;
			}else{
			//fprintf(stderr,"size:%zd,p_size:%zd\n",p->size,p->next->size-1);
			//fprintf(stderr,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
				size_t sub = size - (p->size & ~1);
				meta_data *new = (void*)p->next + sub;
				//debug
				memmove(new,p->next,sizeof(meta_data));
				new->size = new->size - sub;
				new->ptr = new + 1;
				new->next->previous = new;
				p->next = new;
				p->size = size & ~1;
				//debug
				/*new->size = p->next->size - sub;
				new->next = p->next->next;
				new->previous = p;
				new->ptr = new + 1;
				p->next->next->previous = new;
				p->next = new;
				p->size = size & ~1;*/			
				//fprintf(stderr,"size:%zd,newsize:%zd\n",p->size,new->size-1);
			}
			//fprintf(stderr,"end:%p\n",new->ptr);
		}else if (p->next == head){
			sbrk(size - (p->size & ~1));
			p->size = size & ~1;
		}else{
			//fprintf(stderr,"No biger size or next not free \n");
			//fprintf(stderr,"size:%zd,p_size:%zd,%p\n",p->size,p->next->size,p->next);
			void* result = malloc(size);
			memmove(result,ptr,p->size & ~1);
			free(ptr);
			return result;
				
		}
	}
	else if (size < p->size){
		//fprintf(stderr,"smaller_r\n");
		if (p->next!= head && (p->next->size & 1)){		// next is freed
			/*size_t sub = p->size - size;
			meta_data *new = (void*)p->next - sub;
			new->size = p->next->size + sub;
			new->next = p->next->next;
			new->ptr = new + 1;
			new->previous = p;
			p->next->next->previous = new;
			p->next = new;
			p->size = size & ~1;*/
			/*meta_data *p_search = p->next->next;
			meta_data *chosen = NULL;
			while(p_search!= NULL){
				if (size > maxsize) break;
				if ((p_search->size & 1)&& (p_search->size & ~1) >= size){  //encode free in size
					if (chosen == NULL || (chosen && p_search->size < chosen->size)){
						 chosen = p_search ;
						// break;
					}
				}
				if (p_search == p->next) break;
				p_search = p_search->next;
			}
			if (chosen == p->next){
				size_t sub = p->size - size;
				meta_data *new = (void*)p->next - sub;
				new->size = p->next->size + sub;
				new->next = p->next->next;
				new->ptr = new + 1;
				new->previous = p;
				p->next->next->previous = new;
				p->next = new;
				p->size = size & ~1;
			}else{
				void *result = malloc(size);
				memmove(result,ptr,p->size);			
				free(ptr);
				return result;
			}*/
			free(ptr);
			reverse_search = 1;
			void *result = malloc(size);
			if (result != ptr)
				memmove(result,ptr,size);
			//fprintf(stderr,"result:%p\n\n",result);			
			return result;
		}else if (p->next!= head && p->size > size + sizeof(meta_data)+15){
			/*size_t original_size = p->size;
			p->size = size;
			meta_data *new = p->ptr + size;
			new->ptr = new + 1;	
			new->size = original_size - size - sizeof(meta_data);
			new->size = new->size | 1;
			new->previous = p;
			new->next = p->next;
			p->next->previous = new;
			p->next = new;*/
			free(ptr);
			reverse_search = 1;
			void *result = malloc(size);
			if (result != ptr)
				memmove(result,ptr,size);	
			//fprintf(stderr,"result:%p\n\n",result);
			return result;
		}else{
			free(ptr);
			reverse_search = 1;
			void *result = malloc(size);
			if (result != ptr)
				memmove(result,ptr,size);	
			//fprintf(stderr,"result:%p\n\n",result);
			return result;
			//p->size = size & ~1;
		}
	}
    return ptr;
}
