#include<stdio.h>
#include<stdlib.h>
int main(){
	void *top_of_heap = sbrk(0);
//	malloc(65536*16);
	void *top_of_heap2 = sbrk(65536*16*16*16);
	void *top_of_heap3 = sbrk(0);
	printf("The top of heap went from %p to %p \n", top_of_heap2, top_of_heap3);
//	size_t size = 4190001;
//	size = (size + 15)/16*16;
//	printf("%zd\n",size & ~1);
//	char *p = malloc (20);
//	void *p1 = malloc (20);
///	free (p);
//	void *p2 = calloc (1,10);
//	free (p2);
//	void *p3 = malloc (4190001);
//	void *p4 = malloc ();
	return 0;
}
