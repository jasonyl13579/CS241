/**
* Text Editor Lab
* CS 241 - Spring 2018
*/

#include "sstream.h"

int main(int argc, char *argv[]) {
    // TODO create some tests
	bytestring b = (bytestring)  {NULL,6};
	bytestring b1 =(bytestring) {"123",3};
	bytestring b2 =(bytestring) {"Hello ",-1};
	sstream* s = sstream_create(b);
	//sstream* s1 = sstream_create(b1);
	bytestring c;
	c.size = -1;
	c.str = "world";
	sstream_str(s,c);
	sstream_str(s,b);
	sstream_str(s,b1);
	sstream_str(s,b2);
	sstream_peek(s,0);
	bytestring d;
	d.size = -1;
	d.str = "test-9223372036854775807";
	sstream_str(s,d);
//	sstream_write(s,d);
//	int seek = sstream_seek(s,2, SEEK_SET);
	//fprintf(stderr,"seek:%d\n",seek);
	/*sstream_insert(s,c);
	seek = sstream_seek(s,3, SEEK_SET);
	bytestring e;
	e.size = 2;
	e.str = "ld";
	//printf("sub_offset:%zd\n",sstream_subseq(s,e));
	seek = sstream_seek(s,9, SEEK_SET);
	bytestring f;
	f.size = 1;
	f.str = NULL;*/
	sstream_seek(s,4, SEEK_SET);
	long x = 1;
	sstream_parse_long(s,&x);
//	seek = sstream_seek(s,5, SEEK_SET);
/*	sstream_read(s,&f,6);
	sstream_append(s,c);*/
//	sstream_seek(s,5, SEEK_SET);
//	sstream_erase(s,-100);
//	free (f.str);
	sstream_destroy(s);
	
    return 0;
}