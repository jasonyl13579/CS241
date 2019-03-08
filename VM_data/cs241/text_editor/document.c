/**
* Text Editor Lab
* CS 241 - Spring 2018
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "document.h"
#include "vector.h"

struct document {
    vector *vector;
};

document *document_create() {
    document *this = (document *)malloc(sizeof(document));
    assert(this);
    this->vector = vector_create(string_copy_constructor, string_destructor,
                                 string_default_constructor);
    return this;
}

void document_write_to_file(document *this, const char *path_to_file) {
    assert(this);
    assert(path_to_file);
	int f = open(path_to_file, O_RDWR|O_CREAT,S_IXOTH|S_IWOTH|S_IROTH|S_IXUSR|S_IWUSR|S_IRUSR );
	 if( -1 == f ){
        printf( "open failure\n" );
        return ;
    }
	fchmod(f,S_IXOTH|S_IWOTH|S_IROTH|S_IXUSR|S_IWUSR|S_IRUSR);
   // chmod(path_to_file, S_IROTH |S_IWOTH);
	const char* line;
	for (size_t i=1 ; i<= document_size(this) ; i++){
		line = document_get_line(this,i);
		dprintf(f,"%s", line);
		if (i!=document_size(this))
			dprintf(f,"\n");
	}
    // see the comment in the header file for a description of how to do this!
    // TODO: your code here!
}

document *document_create_from_file(const char *path_to_file) {
    assert(path_to_file);
	document* d = document_create();
	FILE *f = fopen(path_to_file, "r" );
	if (f == NULL){
		//puts("test");
	}else{
		char *buffer = NULL;
		size_t size = 0;
		ssize_t chars ;
		while((chars = getline(&buffer, &size, f))!= -1){
			// Discard newline character if it is present,
			//printf("%zd\n",chars);
			if (chars > 0 && buffer[chars-1] == '\n') 
				buffer[chars-1] = '\0';
			vector_push_back(d->vector,buffer);
		}
		free(buffer);
	}
	if(f != NULL)
		fclose(f);
    // this function will read a file which is created by document_write_to_file
    // TODO: your code here!
    return d;
}

void document_destroy(document *this) {
    assert(this);
    vector_destroy(this->vector);
    free(this);
}

size_t document_size(document *this) {
    assert(this);
    return vector_size(this->vector);
}

void document_set_line(document *this, size_t line_number, const char *str) {
    assert(this);
    assert(str);
    size_t index = line_number - 1;
    vector_set(this->vector, index, (void *)str);
}

const char *document_get_line(document *this, size_t line_number) {
    assert(this);
    assert(line_number > 0);
    size_t index = line_number - 1;
    return (const char *)vector_get(this->vector, index);
}

void document_insert_line(document *this, size_t line_number, const char *str) {
    assert(this);
    assert(str);
	if (str == NULL)return;
	size_t index = line_number - 1 ;
	if (line_number <= document_size(this)){
		vector_insert(this->vector, index, (void *)str);
	}else if (line_number > document_size(this)){
		size_t size = document_size(this);
		for (size_t i=1;i<line_number - size;i++)
			vector_push_back(this->vector,"");
		vector_push_back(this->vector,(void *)str);
	}
	
	//for (size_t i=0;i<document_size(this);i++)
	//		printf("%s\n",document_get_line(this, i+1));
    // TODO: your code here!
    // How are you going to handle the case when the user wants to
    // insert a line past the end of the document?
}

void document_delete_line(document *this, size_t line_number) {
    assert(this);
    assert(line_number > 0);
    size_t index = line_number - 1;
    vector_erase(this->vector, index);
}
