/**
* Pointers Gone Wild Lab
* CS 241 - Spring 2018
*/

#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {
    // your code here
	first_step(81);
	int a = 132;
	second_step(&a);
	int *b = (int*)malloc(4);
	*b = 8942;
	double_step(&b);
	free(b);
	// 
	int ic = 15;
	int *iic = &ic;
	char *c = (char*)iic;
	c = c - 5;
	//char c5 = 0x0F;
	//c[5] = c5;
	strange_step(c);
	//
	void *d = (void*)calloc(10,1);
	empty_step(d);
	free(d);
	char s2[]="uuuuu";
	void *s = (void*)s2;
	two_step(s, s2);
	char* first = (char*)calloc(20,1);
	char* second = first+2;
	char* third = second+2;
	three_step(first,second,third); 
	free(first);
	//
	char* f = (char*)calloc(20,1);
	char* se = f+2;
	char* th = se+2;
	f[0] = '1';
	f[1] = '0';
	se[2]='8';
	th[3]=64;
	step_step_step(f,se,th);
	free(f);
	//
	char *x = "2";
	int y = 2+'0';
	it_may_be_odd(x,y);
	// 
	char cs[] = ",adad,CS241,afaf";
	char *cstest;
	cstest=strdup(cs);
	//printf("%s\n",ccc);
	tok_step(cstest);//test
	free(cstest);
	//
	char orange[] = "1002";
	orange[0] ='\x01';
	void *blue = orange;
	the_end((void*)orange,blue);
	//tok_step(cs);
    return 0;
}
