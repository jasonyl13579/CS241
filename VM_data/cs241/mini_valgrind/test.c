/**
* Mini Valgrind Lab
* CS 241 - Spring 2018
*/

#include <stdio.h>
#include <stdlib.h>

int main() {
    // Your tests here using malloc and free
	void *p1 = malloc(30);
    void *p2 = calloc(30,1);
   // void *p3 = malloc(30);
	free(p2); free(p2);
	// void *p3 = malloc(30);
	//p2 = realloc(p2,40); //if (p4 ==NULL) puts("1");
	//p2 = realloc(p2,50);
	//p2 = realloc(p2,40);
	//p2 = realloc(p2,30);
   // free(p2);
//	p1 = realloc(p1,20);
//	free(p3);
//	p1 = realloc(p1,40);
//	 free(p4);
//	free(p3);
//	free(p1);
	free(p1);
//	 free(p4);
	 
    return 0;
}
