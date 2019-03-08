/**
* Networking Lab
* CS 241 - Spring 2018
*/

#include "common.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "error.h"
#include "errno.h"
static const size_t MESSAGE_SIZE_DIGITS = 8;


ssize_t get_message_size(int socket) {
    int64_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;
	//printf("size:%s\n",(char*)&size);
	//printf("read:%d,%zd\n",size,(ssize_t)ntohl(size));
    return size;
}

// You may assume size won't be larger than a 4 byte integer
ssize_t write_message_size(int64_t size, int socket) {
    // Your code here
	//size_t buf = htonl(size);
    ssize_t write_bytes =
        write_all_to_socket(socket, (char*)&size, MESSAGE_SIZE_DIGITS);
    if (write_bytes == 0 || write_bytes == -1)
        return write_bytes;
	//printf("write:%zu,%zd\n",size,(ssize_t)htonl(size));
    return size;
}

ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    // Your Code Here 
	int finished = 0;
	int len = 0;
	//count ++ ;
	int start = 1;
	while (1){		
		len = read(socket, (void*)(buffer + finished), count - finished);
		//printf("read_len:%d\n",len);
		if (len == -1 && errno == EINTR)continue;
		if (len < 0) {
			perror("read");
			if (!start)return finished;
			else return -1;
		}
		if (len == 0) {
			if (start) return 0;
			else return finished;
		}
		start = 0;
		finished += len;
		//printf("finished:%d\n",finished);
		if (finished == (int)count) return finished;
	}
    return count;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    // Your Code Here
	int finished = 0;
	int len = 0;
	int start = 1;
	while (1){
		len = write(socket, buffer + finished, count - finished);
		if (len == -1 && errno == EINTR)continue;
		if (len < 0) {
			perror("write");
			if (!start)return finished;
			else return -1;
		}
		if (len == 0) {
			if (start) return 0;
			else return finished;
		}
		start = 0;
		finished += len;
		//printf("finished:%d\n",finished);
		if (finished == (int)count) return finished;
	}
    return count;
}
size_t read_until_newline(int socket, const char *buffer){
	size_t count = 0;
	int len = 0;
	while (1){
		len = read(socket, (void*)(buffer + count), 1);
		if (len == -1 && errno == EINTR)continue;
		if (len < 0) return -1;
		if (len == 0) {
			return count;
		}
		if (*(buffer + count) == '\n')break;
		count++;		
	}
	count++;
	//LOG("Receive: %zd bytes",count);
	return count;
}