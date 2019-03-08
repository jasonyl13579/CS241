/**
* Extreme Edge Cases Lab
* CS 241 - Spring 2018
*/

#include "camelCaser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

char **camel_caser(const char *input_str) {
	if (*input_str == 0) return NULL;
	int punct_count = 0;
	char **output = 0;
	//char *buf = (char*)malloc(strlen(input_str)+1);
	char buf[strlen(input_str)+1];
	const char *cur = input_str; 
	while (*cur){
		if (ispunct(*cur))
			punct_count ++;
		cur++;
	}
	cur = input_str;
	int count = 1, row_count = 0;
	int first = 1 , space = 0;
	output = (char**)malloc((punct_count+1)*sizeof(char*));
	char** o = output;
	while (*cur){
		if (ispunct(*cur)){
			output[row_count] = (char*)malloc(sizeof(char)*count);
			buf[count-1] ='\0';
			//char *word = *output;
			strcpy(*o,buf);
			printf("%s\n",*o);
			o++;
			count ++;	
			row_count++;
			count = 1;
			first = 1;
			//buffer = realloc(buf, count *sizeof(char));
		}else{
			if (!isspace(*cur)){
			
				if (first==1) {
					buf[count-1] = tolower(*cur);
					first = 0;
				}else if (space==1){
					buf[count-1] = toupper(*cur);
					space = 0;
				}else {
					buf[count-1] = tolower(*cur);
				}
				count++; 
				space = 0;
			}else {space = 1; }
		}
		cur++;
	}
	o = NULL;
    return output;
}

void destroy(char **result) {
	char** r = result;
	int count = 1;
	while(*r++)count++;
	//printf ("%d\n",count);
	for(int i=0;i<count;i++)free(result[i]);
	free (result);
    return;
}
