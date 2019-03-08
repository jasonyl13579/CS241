/**
* Finding Filesystems Lab
* CS 241 - Spring 2018
*/

#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
int test_write_1(file_system *fs);
int test_write_2(file_system *fs);
int test_write_3(file_system *fs);
int test_write_4(file_system *fs);
int test_write_5(file_system *fs);
int main(int argc, char *argv[]) {
    file_system *fs = open_fs("test.fs");
    if (fs == NULL) {
        exit(1);
    }
	if (argc != 2){
		return 1;
	}
	int index = atoi(argv[1]);
	switch(index){
		case 1:
			test_write_1(fs);
			break;
		case 2:
			test_write_2(fs);
			break;	
		case 3:
			test_write_3(fs);
			break;	
		case 4:
			test_write_4(fs);
			break;	
		case 5:
			test_write_5(fs);
			break;	
	}
}

int test_write_1(file_system *fs) {
    char *path = "/goodies/test1.txt";
    size_t count = 1024;
    char *buf = malloc(count);
	int i = 0;
	while (i != (int)count){
		*(buf + i) = 'a'+ i%5 ;
		i++;
	}
	buf[count-1] = '\0';
	//printf("buf:%s\n",buf);
    off_t off = 0;

    ssize_t bytes_write = minixfs_write(fs, path, buf, count, &off);
    if (bytes_write == -1) {
        perror("test_write");
        exit(1);
    } else {
		off = 0;
		char *ans = malloc (count);
		ssize_t bytes_read = minixfs_read(fs, path, ans, count, &off);
		//printf("ans:%s\n",ans);
		if (strcmp(buf,ans) == 0)
			printf("success\n");
		else
			printf("Fail\n");
		printf("write: %ld bytes\n",bytes_write);
		printf("read: %ld bytes\n",bytes_read);
        //test_read_1(fs);
    }
    return 0;
}
int test_write_2(file_system *fs) {
    char *path = "/goodies/test2.txt";
   // size_t count = sizeof(data_block)* NUM_DIRECT_INODES + 100;
    size_t count = 30;
    char *buf = malloc(count);
	int i = 0;
	while (i != (int)count){
		*(buf + i) = 'a'+ i%5 ;
		i++;
	}
	buf[count-1] = '\0';
	printf("buf:%s\n",buf);
    off_t off = 10;

    ssize_t bytes_write = minixfs_write(fs, path, buf, count, &off);
    if (bytes_write == -1) {
        perror("test_write");
        exit(1);
    } else {
		off = 10;
		char *ans = malloc (count);
		ssize_t bytes_read = minixfs_read(fs, path, ans, count, &off);
		
		off = 0;
		printf("off:%ld bytes\n",off);
		printf("buf:%s\n",ans);
		if (strcmp(buf+off,ans) == 0)
			printf("success\n");
		else
			printf("Fail\n");
		printf("write: %ld bytes\n",bytes_write);
		printf("read: %ld bytes\n",bytes_read);
        //test_read_1(fs);
    }
    return 0;
}
int test_write_3(file_system *fs) {
    char *path = "/goodies/test3.txt";
   // size_t count = sizeof(data_block)* NUM_DIRECT_INODES + 100;
    size_t count = sizeof(data_block)* (NUM_DIRECT_INODES + NUM_INDIRECT_INODES -2) + 100;
    char *buf = malloc(count);
	printf("count:%zd\n",count);
	int i = 0;
	while (i != (int)count){
		*(buf + i) = 'a'+ i%5 ;
		i++;
	}
	buf[count-1] = '\0';
    off_t off = 0;
	//printf("%s\n",buf);
    ssize_t bytes_write = minixfs_write(fs, path, buf, count, &off);
    if (bytes_write == -1) {
        perror("test_write");
        exit(1);
    } 
	return 0;
}
int test_write_4(file_system *fs) {
    char *path = "/goodies/test4.txt";
    size_t count = sizeof(data_block)* NUM_DIRECT_INODES + 100;
    // size_t count = 30;
    char *buf = malloc(count);
	int i = 0;
	while (i != (int)count){
		*(buf + i) = 'a'+ i%5 ;
		i++;
	}
	buf[count-1] = '\0';
	//printf("buf:%s\n",buf);
    off_t off = 10;

    ssize_t bytes_write = minixfs_write(fs, path, buf, count, &off);
    if (bytes_write == -1) {
        perror("test_write");
        exit(1);
    } else {
		off = 10;
		char *ans = malloc (count);
		ssize_t bytes_read = minixfs_read(fs, path, ans, count, &off);
		
		off = 0;
		printf("off:%ld bytes\n",off);
	//	printf("buf:%s\n",ans);
		if (strcmp(buf+off,ans) == 0)
			printf("success\n");
		else
			printf("Fail\n");
		printf("write: %ld bytes\n",bytes_write);
		printf("read: %ld bytes\n",bytes_read);
        //test_read_1(fs);
    }
    return 0;
}
int test_write_5(file_system *fs) {
    char *path = "/goodies/test5.txt";
    //size_t count = sizeof(data_block)* NUM_DIRECT_INODES + 100;
     size_t count = 30;
    char *buf = malloc(count);
	int i = 0;
	while (i != (int)count){
		*(buf + i) = 'a'+ i%5 ;
		i++;
	}
	buf[count-1] = '\0';
	//printf("buf:%s\n",buf);
    off_t off = sizeof(data_block)* NUM_DIRECT_INODES + 100;

    ssize_t bytes_write = minixfs_write(fs, path, buf, count, &off);
    if (bytes_write == -1) {
        perror("test_write");
        exit(1);
    } else {
		off = sizeof(data_block)* NUM_DIRECT_INODES + 100;
		char *ans = malloc (count);
		ssize_t bytes_read = minixfs_read(fs, path, ans, count, &off);
		
	//	printf("buf:%s\n",ans);
		if (strcmp(buf,ans) == 0)
			printf("success\n");
		else
			printf("Fail\n");
		printf("write: %ld bytes\n",bytes_write);
		printf("read: %ld bytes\n",bytes_read);
        //test_read_1(fs);
    }
    return 0;
}