#include<stdio.h>
#include<stdlib.h>
int main(){
	/*void *top_of_heap = sbrk(0);
	//malloc(16384);
	void *top_of_heap2 = sbrk(10);
	printf("The top of heap went from %p to %p \n", top_of_heap, top_of_heap2);*/
//	size_t size = 65;
//	size = (size + 15)/16*16;
//	printf("%zd\n",size | 1);
	void *p1 = malloc (16);
	void *p = malloc (2147483616);
	
	//void *p3 = malloc (64);
//	free (p1);
//	free (p);
	//void *p4 = realloc (p3,16);
//	free (p);
//	void *p2 = calloc (1,10);
//	free (p2);
//	void *p3 = realloc (64);
//	void *p4 = malloc ();
	return 0;
}
