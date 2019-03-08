/**
* Chatroom Lab
* CS 241 - Spring 2018
*/

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"
#include "user_hooks.h"
#include "error.h"
#include "errno.h"
static const size_t MESSAGE_SIZE_DIGITS = 4;

char *create_message(char *name, char *message) {
    int name_len = strlen(name);
    int msg_len = strlen(message);
    char *msg = calloc(1, msg_len + name_len + 4);
    sprintf(msg, "%s: %s", name, message);

    return msg;
}

ssize_t get_message_size(int socket) {
    int32_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;
	//printf("read:%d,%zd\n",size,(ssize_t)ntohl(size));
    return (ssize_t)ntohl(size);
}

// You may assume size won't be larger than a 4 byte integer
ssize_t write_message_size(size_t size, int socket) {
    // Your code here
	size_t buf = htonl(size);
    ssize_t write_bytes =
        write_all_to_socket(socket, (char*)&buf, MESSAGE_SIZE_DIGITS);
    if (write_bytes == 0 || write_bytes == -1)
        return write_bytes;
	//printf("write:%zu,%zd\n",size,(ssize_t)htonl(size));
    return (ssize_t)htonl(size);
}

ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    // Your Code Here 
	//printf("read count:%zd\n",count);
	int finished = 0;
	int len = 0;
	while (1){
		len = read(socket, buffer + finished, count - finished);
		if (len == -1 && errno == EINTR)continue;
		if (len < 0) return -1;
		if (len == 0) return 0;
		finished += len;
		if (finished == (int)count) return finished;
	}
    return count;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    // Your Code Here
	int finished = 0;
	int len = 0;
	while (1){
		len = write(socket, buffer + finished, count - finished);
		if (len == -1 && errno == EINTR)continue;
		if (len < 0) return -1;
		if (len == 0) return 0;
		finished += len;
		if (finished == (int)count) return finished;
	}
    return count;
}
