/**
* Text Editor Lab
* CS 241 - Spring 2018
*/

#include "document.h"
#include "editor.h"
#include "format.h"
#include "sstream.h"

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CSTRING(x) ((bytestring){(char *)x, -1})
#define DECLARE_BUFFER(name) bytestring name = {NULL, 0};
#define EMPTY_BYTES ((bytestring){NULL, 0})

char *get_filename(int argc, char *argv[]) {
    // TODO implement get_filename
    // take a look at editor_main.c to see what this is used for
	if (argc == 2)	return argv[1];
	else return "";
}

sstream *handle_create_stream() {
    // TODO create empty stream
	sstream* s = sstream_create(EMPTY_BYTES); 
    return s;
}
document *handle_create_document(const char *path_to_file) {
    // TODO create the document
	document* d = document_create_from_file(path_to_file);
    return d;
}

void handle_cleanup(editor *editor) {
    // TODO destroy the document
	document_destroy(editor->document);
}

void handle_display_command(editor *editor, size_t start_line,
                            ssize_t max_lines, size_t start_col_index,
                            ssize_t max_cols) {
    // TODO implement handle_display_command
	if (start_line == 0 || document_size(editor->document) == 0){
		print_document_empty_error();
		return;
	}
	if (max_lines == -1) start_line = 1;
	if( document_size(editor->document)< (size_t)max_lines || max_lines == -1)
		max_lines = document_size(editor->document);	
	for (size_t index = start_line ; index <= (size_t)max_lines ; index++)
		print_line(editor->document, index, start_col_index , max_cols);
}

void handle_insert_command(editor *editor, location loc, const char *line) {
    // TODO implement handle_insert_command
	bytestring bytes;
	if (document_size(editor->document) >= loc.line_no){
		bytes.str = strdup(document_get_line(editor->document,loc.line_no));
		bytes.size = strlen(document_get_line(editor->document,loc.line_no));
		sstream_str(editor->stream , bytes);
		bytes.str = strdup(line);
		//fprintf(stderr,"h_insert:%s\n",line);
		bytes.size = strlen(line);
		sstream_seek(editor->stream ,loc.idx, SEEK_SET);
		sstream_insert(editor->stream , bytes);
		sstream_seek(editor->stream ,0, SEEK_SET);
		sstream_read(editor->stream, &bytes ,sstream_size(editor->stream));
		document_set_line(editor->document,loc.line_no, bytes.str );
		free(bytes.str);
	}else{
		for (size_t i= document_size(editor->document)+1; i<loc.line_no;i++){
			document_insert_line(editor->document, i, "");
		}
		document_insert_line(editor->document, loc.line_no , line);
		
	}
	//fprintf(stderr,"h_insert:%s\n",line);
}

void handle_append_command(editor *editor, size_t line_no, const char *line) {
    // TODO implement handle_append_command
	bytestring bytes;
	bytestring out ={NULL,0};
	if (document_size(editor->document) >= line_no){
		bytes.str = strdup(document_get_line(editor->document,line_no));
		bytes.size = strlen(document_get_line(editor->document,line_no));
	//	char* byte_str = bytes.str + bytes.size + 1;	
		sstream_str(editor->stream , bytes);
		sstream_seek(editor->stream, 0, SEEK_END);	
		bytes.str = strdup(line);
		//fprintf(stderr,"%s\n",line);
		bytes.size = strlen(line);		
		sstream_insert(editor->stream ,bytes);
	//debug
	}else if (line_no > 0){
		bytes.str = strdup(line);
		//fprintf(stderr,"%s\n",line);
		bytes.size = strlen(line);		
		sstream_str(editor->stream ,bytes);
	}
	if (line_no > 0){
		size_t offset = 0;
		bytes.str[0] = '\\';
		bytes.size = 1;	
		int newline = 0;
		while((offset = (size_t)sstream_subseq(editor->stream , bytes))!= (size_t)-1){
			
			sstream_seek(editor->stream ,offset, SEEK_CUR);
			sstream_erase(editor->stream ,1);
			if(sstream_peek(editor->stream,0) == '\\') sstream_seek(editor->stream ,1, SEEK_CUR);
			else if (sstream_peek(editor->stream,0) == 'n'){
				//fprintf(stderr,"change\n");
				sstream_erase(editor->stream ,1);
				
				size_t position = sstream_tell(editor->stream);
				sstream_seek(editor->stream ,0, SEEK_SET);
				sstream_read(editor->stream, &out ,position);
				sstream_seek(editor->stream ,0, SEEK_SET);
				sstream_erase(editor->stream ,position);
				printf("%zd\n",position);
				if (!newline && document_size(editor->document) > line_no){
					document_set_line(editor->document,line_no, out.str );
					line_no++;
					//document_insert_line(editor->document,line_no, out.str );
					newline = 1;				
				}
				else{
				//	sstream_read(editor->stream, &out ,position);
				//	sstream_seek(editor->stream ,0, SEEK_SET);
				//	fprintf(stderr,"for_line = %zd\n",line_no);
					document_insert_line(editor->document,line_no, out.str );
					line_no++;
				}
			}
		//	fprintf(stderr,"p:%zd\n",sstream_tell(editor->stream ));
		}
		sstream_seek(editor->stream ,0, SEEK_SET);
		sstream_read(editor->stream, &out ,sstream_size(editor->stream));
		sstream_seek(editor->stream ,0, SEEK_SET);
		if(!newline && document_size(editor->document) > line_no)	
			document_set_line(editor->document,line_no, out.str );
		else	document_insert_line(editor->document,line_no, out.str );
		
		free(bytes.str);
		if (out.str != NULL)
			free(out.str);
	}
	
}

void handle_write_command(editor *editor, size_t line_no, const char *line) {
    // TODO implement handle_write_command
	bytestring bytes;
	bytestring out ={NULL,0};
//	if (document_size(editor->document) >= line_no){
	if (line_no >0){
		bytes.str = strdup(line);
		//fprintf(stderr,"%s\n",line);
		bytes.size = strlen(line);		
		sstream_str(editor->stream ,bytes);
		size_t offset = 0;
		bytes.str[0] = '\\';
		bytes.size = 1;	
		int newline = 0;
		while((offset = (size_t)sstream_subseq(editor->stream , bytes))!= (size_t)-1){
			
			sstream_seek(editor->stream ,offset, SEEK_CUR);
			sstream_erase(editor->stream ,1);
			if(sstream_peek(editor->stream,0) == '\\') sstream_seek(editor->stream ,1, SEEK_CUR);
			else if (sstream_peek(editor->stream,0) == 'n'){
				//fprintf(stderr,"change\n");
				sstream_erase(editor->stream ,1);
				
				size_t position = sstream_tell(editor->stream);
				sstream_seek(editor->stream ,0, SEEK_SET);
				sstream_read(editor->stream, &out ,position);
				sstream_seek(editor->stream ,0, SEEK_SET);
				sstream_erase(editor->stream ,position);
				if (!newline && document_size(editor->document) > line_no){
					document_set_line(editor->document,line_no, out.str );
					newline = 1;
				}else{
				//	fprintf(stderr,"line = %zd\n",line_no);
					document_insert_line(editor->document,line_no, out.str );
				}
				line_no++;

			}
		//	fprintf(stderr,"p:%zd\n",sstream_tell(editor->stream ));
		}
		sstream_seek(editor->stream ,0, SEEK_SET);
		sstream_read(editor->stream, &out ,sstream_size(editor->stream));
		sstream_seek(editor->stream ,0, SEEK_SET);
	//	fprintf(stderr,"line = %zd,out:%s\n",line_no,out.str);
		if(!newline && document_size(editor->document) > line_no) 	
			document_set_line(editor->document,line_no, out.str );
		else	document_insert_line(editor->document,line_no, out.str );
		free(bytes.str);
		if (out.str != NULL)
			free(out.str);

	}
	
}

void handle_delete_command(editor *editor, location loc, size_t num_chars) {
    // TODO implement handle_delete_command
	bytestring bytes , out = EMPTY_BYTES;
	if (loc.line_no > document_size(editor->document)){
		for (size_t i=1 ; i<document_size(editor->document);i++)
			document_delete_line(editor->document, i);
		return ;
	}
	bytes.str = strdup(document_get_line(editor->document,loc.line_no));
	bytes.size = strlen(document_get_line(editor->document,loc.line_no));	
	sstream_str(editor->stream , bytes);
	//sstream_read(editor->stream, &bytes ,sstream_size(editor->stream));
	if (strlen(bytes.str) - loc.idx <= num_chars)
		document_delete_line(editor->document, loc.line_no);
	else{
		sstream_seek(editor->stream ,loc.idx, SEEK_SET);
		sstream_erase(editor->stream, num_chars);
		sstream_seek(editor->stream ,0, SEEK_SET);
		sstream_read(editor->stream, &out ,sstream_size(editor->stream));
		document_set_line(editor->document, loc.line_no,out.str);
	}
		
	free(bytes.str);
}

void handle_delete_line(editor *editor, size_t line_no) {
    // TODO implement handle_delete_line
	if (line_no <= document_size(editor->document))
		document_delete_line(editor->document, line_no);
}

location handle_search_command(editor *editor, location loc,const char *search_str) {
    // TODO implement handle_search_command
	if (!strcmp(search_str ,""))
		return (location){0, 0};
	bytestring bytes, search;
	search.str = strdup(search_str);
	search.size = strlen(search_str);
	bytes.str = strdup(document_get_line(editor->document,loc.line_no));
	bytes.size = strlen(document_get_line(editor->document,loc.line_no));	
	sstream_str(editor->stream , bytes);
	sstream_seek(editor->stream, loc.idx,SEEK_SET);
	ssize_t offset = 0;
	int find = 0;
	size_t i = 0;
//	puts("test");
	for (i = loc.line_no; i <= document_size(editor->document);i++){
		offset = sstream_subseq(editor->stream , search);
		if (offset != -1) {
			if (i == loc.line_no) offset += loc.idx;
			found_search(search.str, i, offset);
			fflush(stderr);
			find = 1; break;
		}			
		if (i== document_size(editor->document))break;
		bytes.str = strdup(document_get_line(editor->document,i+1));
		bytes.size = strlen(document_get_line(editor->document,i+1));	
		sstream_str(editor->stream , bytes);
		sstream_seek(editor->stream , 0 , SEEK_SET);
	}
	//free (search.str);
	//free (bytes.str);
	if(!find) {
		not_found( search.str);
		return (location){0, 0};
	}
	else return (location){i,offset};
}

void handle_merge_line(editor *editor, size_t line_no) {
    // TODO implement handle_merge_line
	if (line_no >= document_size(editor->document)|| line_no <=0)return;
	bytestring bytes, out = EMPTY_BYTES;
	bytes.str = strdup(document_get_line(editor->document,line_no));
	bytes.size = strlen(document_get_line(editor->document,line_no));	
	sstream_str(editor->stream , bytes);
	bytes.str = strdup(document_get_line(editor->document,line_no+1));
	bytes.size = strlen(document_get_line(editor->document,line_no+1));
	sstream_seek(editor->stream,0,SEEK_END);	
	sstream_insert(editor->stream , bytes);
	sstream_seek(editor->stream ,0, SEEK_SET);
	sstream_read(editor->stream, &out ,sstream_size(editor->stream));
	document_set_line(editor->document, line_no,out.str);
	document_delete_line(editor->document, line_no+1);
	free(bytes.str);
}

void handle_split_line(editor *editor, location loc) {
    // TODO implement handle_split_line
	bytestring bytes, out = EMPTY_BYTES;
	bytes.str = strdup(document_get_line(editor->document,loc.line_no));
	bytes.size = strlen(document_get_line(editor->document,loc.line_no));	
	sstream_str(editor->stream , bytes);
	sstream_seek(editor->stream ,loc.idx, SEEK_SET);
	sstream_read(editor->stream, &out ,sstream_remain(editor->stream));
	sstream_seek(editor->stream ,loc.idx, SEEK_SET);
	sstream_erase(editor->stream, sstream_remain(editor->stream));
	document_insert_line(editor->document, loc.line_no+1,out.str);
	sstream_seek(editor->stream ,0, SEEK_SET);
	sstream_read(editor->stream, &out ,sstream_remain(editor->stream));
	document_set_line(editor->document, loc.line_no,out.str);
	free(bytes.str);
}

void handle_save_command(editor *editor) {
    // TODO implement handle_save_command
	document_write_to_file(editor->document, editor->filename);
}
