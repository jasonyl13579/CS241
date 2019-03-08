/**
* Networking Lab
* CS 241 - Spring 2018
*/

#pragma once
#include <stddef.h>
#include <sys/types.h>
#define MAX_CHUNK_SIZE 1024
#define LOG(...)                      \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n");        \
    } while (0);

typedef enum { GET, PUT, DELETE, LIST, V_UNKNOWN, V_UNDEFINE } verb;
typedef enum { SERVER, INITIAL, HEADER_PARSED, START, OK, ERROR, RESPONSE, FINISH } state_type;
ssize_t get_message_size(int socket);
ssize_t write_message_size(int64_t size, int socket);
ssize_t read_all_from_socket(int socket, char *buffer, size_t count);
ssize_t write_all_to_socket(int socket, const char *buffer, size_t count);
size_t read_until_newline(int socket, const char *buffer);