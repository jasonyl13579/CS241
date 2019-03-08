/**
* Text Editor Lab
* CS 241 - Spring 2018
*/

#include "editor.h"
#include "format.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define EMPTY_BYTES ((bytestring){NULL, 0})

/**
 * You can programatically test your text editor.
*/
void find_after_writing_test(editor* edi){
	document_insert_line(edi->document,2,"hi");
	handle_search_command(edi, (location) {1,1},"hi");
}
void loading_saving_test(editor* edi){
	document_insert_line(edi->document,2,"hi");
	document_write_to_file(edi->document,edi->filename); 
}
void delete_after_writing_test(editor* edi){
	document_insert_line(edi->document,2,"hi");
	document_delete_line(edi->document,2);
}
void split_test(editor* edi){
	handle_split_line(edi, (location) {1,2});
}
int main() {
    // Setting up a docment based on the file named 'filename'.
    char *filename = "test.txt";
    document *doc = document_create_from_file(filename);
	sstream *sstr = sstream_create(EMPTY_BYTES);
	editor *edi = (editor*)malloc(sizeof(editor));
	edi->document = doc;
    edi->stream = sstr;
    edi->filename = filename;
	//print_line(doc, 2, 0, -1);
//	delete_after_writing_test(edi);
//	find_after_writing_test(edi);
//	loading_saving_test(edi);
//	split_test(edi);
//	document_insert_line(doc,8,"test");
	handle_append_command(edi,1, "\\nwrite\\\\ntext\\\\\\n132");
	handle_append_command(edi,1, "write2\\ntext2");
	handle_append_command(edi,10, "notXXX_AT");
//	handle_write_command(edi,1, "\\nwrite\\\\ntext\\\\\\n132");
//	handle_write_command(edi,1, "write2\\ntext2");
//	handle_write_command(edi,9, "notXXX_AT");
//	handle_merge_line(edi, 0);
	//handle_split_line(edi, (location) {1,5});
	//for (size_t i=0;i<document_size(doc);i++)
	//		printf("%s\n",document_get_line(doc, i+1));
	
	handle_display_command(edi, 2,-1, 0,100);
	//document_write_to_file(doc,filename); 
	document_destroy(doc);

    (void)doc;
	 
	 return 0;
}

