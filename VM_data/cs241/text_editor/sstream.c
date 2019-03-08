/**
* Text Editor Lab
* CS 241 - Spring 2018
*/
#include <assert.h>
#include "sstream.h"
#include "string.h" 
#include "stdlib.h"
#include <ctype.h>
#define min(a,b) ((a)<(b) ? (a) : (b))
typedef struct sstream {
	ssize_t position;
	char* str;
	ssize_t size;
}sstr;
sstream *sstream_create(bytestring bytes) {
    // TODO implement
	sstr* ss = 0; 
	ss = (sstr*)malloc(sizeof(sstr));
	if (bytes.str != NULL && bytes.size < 0 ){
	//	char* str = bytes.str;
	//	ss = (sstream*)malloc((int)strlen(bytes.str));
	//	ss->str = (char*)malloc((int)strlen(bytes.str)+1);
		//need test
		//char* str = ss->str;
		ss->str = strdup(bytes.str);
		ss->position = 0;
		ss->size =(ssize_t)strlen(bytes.str);
	}else if(bytes.str != NULL && bytes.size > 0 ){
		//ss = (sstr*)malloc(bytes.size);
		
		ss->str = (char*)malloc(bytes.size);
		char* str = (char*)ss->str;
		ss->position = 0;
		ss->size = bytes.size;
		for (ssize_t i=0 ; i< bytes.size ; i++)
			str[i] = bytes.str[i];
	}else{
		ss->str = calloc(1,1);
		ss->position = 0;
		ss->size = 0;
	}
	//fprintf(stderr,"create:%s,s:%zd,p:%zd\n",ss->str,ss->size,ss->position);
    return ss;
}

void sstream_destroy(sstream *this) {
	assert(this);
	if(this !=NULL){
		if (this->str != NULL)
			free(this->str);
		free(this);
	}
    // TODO implement
}

void sstream_str(sstream *this, bytestring bytes) {
    // TODO implement
	assert(this);
	if (bytes.str != NULL && bytes.size < 0 ){
	//	ss = (sstream*)malloc((int)strlen(bytes.str));
		//this->str = (char*)realloc(this->str,(int)strlen(bytes.str));
		//this->str = bytes.str;
		if (this->str != NULL)
			free (this->str);
		this->str = strdup(bytes.str);
		this->position = 0;
		this->size = (ssize_t)strlen(bytes.str);
	}else if(bytes.str != NULL && bytes.size > 0 ){
		//ss = (sstr*)malloc(bytes.size);
		
		this->str = (char*)realloc(this->str,bytes.size);
		//*this->str = '0';
		char* str = this->str;
		this->position = 0;
		this->size = bytes.size;
		for (ssize_t i=0 ; i< bytes.size ; i++)
			str[i] = bytes.str[i];
	
	}else{
		this->str = realloc(this->str,1);
		*this->str = 0;
		this->position = 0;
		this->size = 0;
	}
	//fprintf(stderr,"str:%s,s:%zd,p:%zd\n",this->str,this->size,this->position);
}

bool sstream_eos(sstream *this) {
    // TODO implement
	if (this->size == this->position) return true;
	else return false;
}

char sstream_peek(sstream *this, ssize_t offset) {
    // TODO implement
	ssize_t p = this->position + offset;
	if ( this->size <= p || p <0 ) return '\0';
	else {
	//	fprintf(stderr,"peek:%s,s:%zd,p:%zd,c:%c\n",this->str,this->size,this->position,this->str[p]);
		return (this->str[p]);	
	}
}

char sstream_getch(sstream *this) {
    // TODO implement
    if(sstream_eos(this)) return '\0';
	else {
		return this->str[this->position++];
	}
}

size_t sstream_size(sstream *this) {
    // TODO implement
    return this->size;
}

size_t sstream_tell(sstream *this) {
    // TODO implement
    return this->position;
}

int sstream_seek(sstream *this, ssize_t offset, int whence) {
    // TODO implement
	ssize_t cur_position;
	if (whence == SEEK_SET)cur_position = offset;
	else if(whence == SEEK_CUR)cur_position = this->position + offset;
	else if(whence == SEEK_END)cur_position = this->size + offset;
	else return -1;
	//fprintf(stderr,"seek:%zd\n",cur_position);
	if ( this->size < cur_position || cur_position <0 )return -1;
	else{
		this->position = cur_position;
		return (int)cur_position;
	}
}

size_t sstream_remain(sstream *this) {
    // TODO implement
    return this->size-this->position;
}

size_t sstream_read(sstream *this, bytestring *out, ssize_t count) {
    // TODO implement
	size_t count_t = 0;
	if (count >= 0 ) {count_t = min(labs(count), (int)sstream_remain(this));}
	else {count_t = min(labs(count), (int)sstream_tell(this));}
	if(out->str == NULL) {
		out->str = (char*)malloc(sizeof(char)*count_t+1);
		out->size = count_t;
	}else{
		if((size_t)out->size <= count_t) {
			out->str = (char*)realloc(out->str,sizeof(char)*count_t+1);
			out->size = count_t;
		}
	}		
	if (count >= 0){
		char* cur = this->str + this->position;
		for (size_t i=0 ; i<count_t ; i++)
			out->str[i] = cur[i];
		out->str[count_t] = '\0';
		this->position += count_t;
	}else{
		char* cur = this->str + this->position - count_t;
		for (size_t i=0 ; i<count_t ; i++)
			out->str[i] = cur[i];
		out->str[count_t] = '\0';
	}
	//fprintf(stderr,"read:%s,s:%zd,p:%zd\n",out->str,out->size,this->position);
    return count_t;
}

void sstream_append(sstream *this, bytestring bytes) {
    // TODO implement
	if (bytes.str != NULL && bytes.size < 0 ){
		
	//	ss = (sstream*)malloc((int)strlen(bytes.str));
		this->str = (char*)realloc(this->str,(int)strlen(bytes.str)+(int)this->size);
		char* str = this->str;
		str += this->size;
		//str = strdup(bytes.str);
		for (size_t i=0 ; i< strlen(bytes.str) ; i++)
			str[i] = bytes.str[i];
		this->size =(ssize_t)strlen(bytes.str)+ this->size ;
	}else if(bytes.str != NULL && bytes.size > 0 ){
		//ss = (sstr*)malloc(bytes.size);
		this->str = (char*)realloc(this->str , bytes.size + this->size);
		char* str = this->str;
		str += this->size;
		this->size = bytes.size + this->size;
		//ssize_t size = bytes.size;
		for (ssize_t i=0 ; i< bytes.size ; i++)
			str[i] = bytes.str[i];
		/*while(size--){
			*str =  *bytes.str;
			str++;
			bytes.str++;
		}*/
	}else return;
	//fprintf(stderr,"append:%s,s:%zd,p:%zd\n",this->str,this->size,this->position);
}

ssize_t sstream_subseq(sstream *this, bytestring bytes) {
    // TODO implement
	char* cur = this->str;
	cur += this->position;
	ssize_t offset = 0;
	if (bytes.str == NULL) return -1; 
	int size = 0;
	if (bytes.size >= 0) size = bytes.size;
	else size = strlen(bytes.str);
	/*if (bytes.str != NULL && bytes.size < 0 ){
		for (size_t i=0 ; i<sstream_remain(this) ; i++	){
			if(*cur == bytes.str[0] && !strcmp(cur , bytes.str))
				return offset;
			else {
				offset++; cur++;
			}
		}
		
	}else if(bytes.str != NULL && bytes.size > 0 ){
		//ssize_t size = bytes.size;*/
	
		for (size_t i=0 ; i<sstream_remain(this) ; i++	){
			if (*cur == bytes.str[0] && size == 1) return offset;
			if (*cur == bytes.str[0]){
				for (int j=1 ; j< size ; j++){
					if (*(cur+j) != *(bytes.str+j) ) break;
					if(j == size -1) return offset;
				}
			}
			cur ++; offset++;
		}
//	}

//	fprintf(stderr,"subseq:%s,s:%zd,offset:%d\n",this->str,this->size,-1);
    return -1;
}

size_t sstream_erase(sstream *this, ssize_t number) {
    // TODO implement
	char* cur = this->str + this->position;
	if (number > 0){
		if ((this->position + number) <= this->size){
			char* end = cur + number;
			/*ssize_t n = number;
			while(n--){
				if (end != NULL){
					*cur = *end; cur++; end++;
				}else break;
			}*/
			memmove(cur,end,sstream_remain(this)-number);
			this->size -= number;
		//	fprintf(stderr,"erase:%s,s:%zd\n",this->str,this->size);
		}else{
			number = sstream_remain(this);
		//	printf("%zd\n",number);
			this->size -= number;
			//fprintf(stderr,"erase:%s%zd\n",this->str,this->size);
		}
	}else{
		if((this->position + number) < 0){
			char* end = this->str;
			memmove(end,cur,sstream_remain(this));
			number = this->position;
			this->position = 0;
			this->size -= number;
		}else{
			char* end = this->str + this->position +number;
			memmove(end,cur,sstream_remain(this));
			this->position += number;
			this->size += number;
		}
	}
//	fprintf(stderr,"erase:%s,s:%zd,p:%zd\n",this->str,this->size,this->position);
	return number;
}

void sstream_write(sstream *this, bytestring bytes) {
    // TODO implement
	char* str = this->str;
	str += this->position;
	if (bytes.str != NULL && bytes.size < 0 ){
	//	ss = (sstream*)malloc((int)strlen(bytes.str));
		if (this->position + (ssize_t)strlen(bytes.str) > this->size){
			this->str = (char*)realloc(this->str,(int)strlen(bytes.str)+(int)this->position);
			str = this->str + this->position;
			this->size =(ssize_t)strlen(bytes.str)+ this->position ;
		}
		//str = strdup( bytes.str);
	//	fprintf(stderr,"%s\n",bytes.str);
		for (size_t i=0 ; i< strlen(bytes.str) ; i++)
			str[i] = bytes.str[i];
	}else if(bytes.str != NULL && bytes.size > 0 ){
		//ss = (sstr*)malloc(bytes.size);
		if  (this->position + bytes.size > this->size){
			this->str = (char*)realloc(this->str , bytes.size + this->position);
			str = this->str + this->position;
			this->size = bytes.size + this->position;
		}
		//ssize_t size = bytes.size;
		for (ssize_t i=0 ; i< bytes.size ; i++)
			str[i] = bytes.str[i];
		/*while(size--){
			*str =  *bytes.str;
			str++;
			bytes.str++;
		}*/
	}else return;
//	fprintf(stderr,"write:%s,s:%zd,p:%zd\n",this->str,this->size,this->position);
}

void sstream_insert(sstream *this, bytestring bytes) {
    // TODO implement
	char* str = this->str;
	str += this->position;
	if (bytes.str != NULL && bytes.size < 0 ){
	//	ss = (sstream*)malloc((int)strlen(bytes.str));
		this->str  = (char*)realloc(this->str,(int)strlen(bytes.str)+(int)this->size);	
		str = this->str + this->position;
		memmove(str+(int)strlen( bytes.str), str ,sstream_remain(this));
		for (size_t i=0 ; i< strlen( bytes.str); i++)
			str[i] = bytes.str[i];
		//strcpy(str , bytes.str);
		this->size =(ssize_t)strlen(bytes.str)+ this->size ;
	}else if(bytes.str != NULL && bytes.size > 0 ){
		//ss = (sstr*)malloc(bytes.size);
		this->str = (char*)realloc(this->str , bytes.size + this->size);
		str = this->str + this->position;
		memmove(str + bytes.size, str ,sstream_remain(this));
		//ssize_t size = bytes.size;
		this->size = bytes.size + this->size;
		for (ssize_t i=0 ; i< bytes.size ; i++)
			str[i] = bytes.str[i];
		/*while(size--){
			*str =  *bytes.str;
			str++;
			bytes.str++;
		}*/
	}else return;
	//fprintf(stderr,"insert:%s,s:%zd,p:%zd\n",this->str,this->size,this->position);
}

int sstream_parse_long(sstream *this, long *out) {
    // TODO implement
	char* cur = this->str;
	cur += this->position;
	int count = 0;
	if (*cur == '-' ){
		cur++; count++; 
		if (cur && isdigit(*cur)) *out = 0;
		else return -1;  
		while (count + this->position < this->size 	&& isdigit(*cur)){
			if ( (10*(*out) + (*cur -'0')) >= 0){
				*out = 10*(*out) + (*cur -'0');
				cur++; count++;
			}else break;
		}
		*out = -1*(*out);
	}else if (isdigit(*cur)){
		*out = 0;
		while ( count + this->position < this->size && isdigit(*cur)){
			if ( (10*(*out) + (*cur -'0')) >= 0){
				*out = 10*(*out) + (*cur -'0');
				cur++; count++;
			}else break;
		}	
		
	}else return -1;
    this->position += count;
	//printf("%s\n",this->str);
	//int count = 0;
	//fprintf(stderr,"parse_long:%s,s:%zd,p:%zd,long:%ld,count:%d\n",this->str,this->size,this->position,*out,count);
	return count;
}
