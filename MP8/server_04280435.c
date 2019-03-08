/**
* Networking Lab
* CS 241 - Spring 2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <vector.h>
#include <sys/stat.h>
#include "common.h"
#include "format.h"
#define MAXEVENTS 100
#define MAX_CHUNK_SIZE 1024
struct _state{
	int fd;
	verb command;
	state_type type;
	char* filename;
	size_t size;
	size_t remain;
	//char* filecontent;
};
typedef struct _state state;
static volatile int serverSocket;
static int efd = 0;
static struct addrinfo hints, *result;
static char* server_path;
static vector* vector_file;
int create_server(char *port);
static volatile int endSession;

static int make_socket_non_blocking(int socket);
int read_message_from_client(struct epoll_event event);
int handle_initial_state(struct epoll_event event);
int handle_parsed_start_state(struct epoll_event event);
int write_message_to_client(struct epoll_event event);
int handle_put_read(struct epoll_event event);
int handle_get_write(struct epoll_event event);
int write_error_to_client(struct epoll_event event, const char* error_msg);
size_t getFilesize(const char* filename);

void close_server() {
	endSession = 1;
    if (shutdown(serverSocket, SHUT_RDWR) != 0) {
        perror("shutdown():");
    }
    close(serverSocket);
	if (server_path != NULL){
		char rm_command[26];
		strncpy (rm_command, "rm -rf ", 7 + 1);
		strncat (rm_command, server_path, strlen(server_path) + 1);
		if (system (rm_command) == -1)
		{
			perror ("Delete error: ");
			exit (EXIT_FAILURE);
		}
	}
}
int start_server(){
	struct epoll_event event;
	struct epoll_event* events;
	int s;
	if((efd = epoll_create1(0)) == -1){
		perror("epoll_create");
		exit(1);
	}
	//event.data.fd = serverSocket;
	event.events = EPOLLIN ;
	state* initialState = malloc(sizeof(state));
	initialState->fd = serverSocket;
	initialState->command = V_UNDEFINE;
	initialState->size = 0;
	initialState->type = SERVER;
	event.data.ptr = (void*)initialState;
	s = epoll_ctl(efd, EPOLL_CTL_ADD, serverSocket, &event);
	
	if(s == -1){
		perror("epoll_ctl");
		exit(1);
	}
	events = malloc( MAXEVENTS*sizeof(event));
	endSession = 0;
	while(endSession == 0)
	{
		int num_ready = epoll_wait(efd, events, MAXEVENTS,-1);
		//if (num_ready == 0) puts("11");
		for(int i=0; i<num_ready; i++)
		{
			state* metadata = events[i].data.ptr;
			if((events[i].events & EPOLLERR))
				//	||(events[i].events & EPOLLHUP))
			{
				/* An error has occured on this fd, or the socket is not
				   ready for reading (why were we notified then?) */
				fprintf(stderr,"epoll error\n");
				close(metadata->fd);
				continue;
			}

			else if(serverSocket == metadata->fd)
			{
				/* We have a notification on the listening socket, which
				   means one or more incoming connections. */
				while(1)
				{
					int client_fd;
					struct sockaddr in_addr;
					socklen_t in_len;
					char hbuf[NI_MAXHOST],sbuf[NI_MAXSERV];
					//puts("....");
					in_len = sizeof(in_addr);
					client_fd = accept(serverSocket, &in_addr, &in_len);
					if(client_fd == -1)
					{
						if((errno == EAGAIN)||
								(errno== EWOULDBLOCK))
						{
							/* We have processed all incoming
							   connections. */
							break;
						}
						else
						{
							perror("accept");
							break;
						}
					}

					s = getnameinfo(&in_addr, in_len,
							hbuf, sizeof (hbuf),
							sbuf, sizeof (sbuf),
							NI_NUMERICHOST | NI_NUMERICSERV);
					if(s == 0)
					{
						printf("Accepted connection on descriptor %d "
								"(host=%s, port=%s)\n",client_fd , hbuf, sbuf);
					}

					/* Make the incoming socket non-blocking and add it to the
					   list of fds to monitor. */
					make_socket_non_blocking(client_fd);

					
					event.events = EPOLLIN;
					//event.data.fd = client_fd;
					state* initialState = malloc(sizeof(state));
					//printf("fdp:%p\n",&event.data.fd);
					//printf("test:%d\n",event.data.fd);
					initialState->command = V_UNDEFINE;
					initialState->size = 0;
					initialState->type = INITIAL;
					initialState->fd = client_fd;
					initialState->filecontent = NULL;
					initialState->filename = NULL;
					event.data.ptr = (void*)initialState;
					
					s = epoll_ctl(efd, EPOLL_CTL_ADD,client_fd,&event);
					if(s == -1)
					{
						perror("epoll_ctl");
						break;
					}
				}
				continue;
			}
			else
			{
				/* We have data on the fd waiting to be read. Read and
				   display it. We must read whatever data is available
				   completely, as we are running in edge-triggered mode
				   and won't get a notification again for the same
				   data. */
				//int done = 0;
				//while(1)
				//{
					ssize_t count;
					count = read_message_from_client(events[i]);
					//count = read(events[i].data.fd, buf, sizeof(buf));
					if(count == -1)
					{ 
					
						if(errno != EAGAIN || errno != EWOULDBLOCK)
						{
							metadata->type = FINISH;
							
						}
						if(errno == EPIPE){
							//metadata->type = FINISH;	
						}
						
					}
					else if(count == 0)
					{
						metadata->type = FINISH;						
					}
					// Write the buffer to standard output 
					//s = write(metadata->fd ,"OK\ntest.txt", 12);
					if(s ==-1)
					{
						perror("write");
					}
					if(metadata->type == FINISH)
					{
						printf("Closed connection on descriptor %d\n",metadata->fd);
						/* Closing the descriptor will make epoll remove it
						   from the set of descriptors which are monitored. */
						close(metadata->fd);
						if (metadata->filename!= NULL) free(metadata->filename);
						//if (metadata->filecontent != NULL) free(metadata->filecontent);
						free(metadata);
					}
				//}			
			}
		}
	}
	free(events);
	return 0;
}
int main(int argc, char **argv) {
    // good luck!
	
	if(argc != 2){
		fprintf(stderr,"Usage: %s [port]\n",argv[0]);
		return -1;
	}
	struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = close_server;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror("sigaction");
        return 1;
    }

    signal(SIGINT, close_server);
	vector_file = vector_create(string_copy_constructor, string_destructor,
                                 string_default_constructor);
	if (create_server(argv[1]) != 0)  return 1;
	
	if (start_server() != 0) {
		close_server();
		return 1;
	}
	return 0;
	
}
int read_message_from_client(struct epoll_event event){
	state* metadata = event.data.ptr;
	if (metadata->type == INITIAL){
		int r = handle_initial_state(event);
		if (r == -1 || r == 0) return r;
	}
	if (metadata->type == HEADER_PARSED || metadata->type == START){
		int r = handle_parsed_start_state(event);
		if (r == -1 || r == 0) return r;
	}
		
	if(metadata->type == OK || metadata->type == RESPONSE)
		write_message_to_client(event);
	return 1;
}
int handle_initial_state(struct epoll_event event){
	state* metadata = event.data.ptr;
	ssize_t count = 0;
	int client_fd = metadata->fd;
	//LOG("Initial");
	char buffer[280];
	count = read_until_newline(client_fd, buffer);
	if (count == 0 || count == -1) return count;
	int index = 0;
	int filename_index = 0;
	char* command = malloc(10);
	char* filename = malloc(256);	
	int end = 0;
	while (index < count){
		if (index < 10 && buffer[index] != ' ' && !end){
			command[index] = *(buffer+index);
		}
		else if (buffer[index] != ' '){
			filename[filename_index] = *(buffer+index);
			filename_index ++;
		}
		if ((buffer[index] == ' ' || buffer[index] == '\n')&& end == 0){
			command[index] = '\0';
			end = 1;
		}
		index ++;
	}
	filename[filename_index-1] = '\0';
	metadata->filename = strdup(filename);
	if (strcmp(command, "LIST") == 0) metadata->command = LIST;
	else if (strcmp(command, "PUT") == 0) metadata->command = PUT;
	else if (strcmp(command, "GET") == 0) metadata->command = GET;
	else if (strcmp(command, "DELETE") == 0) metadata->command = DELETE;
	else metadata->command = V_UNKNOWN;
	metadata->type = HEADER_PARSED;		
	free(command);
	free(filename);
	return 1;
}
int handle_parsed_start_state(struct epoll_event event){
	state* metadata = event.data.ptr;
	LOG("Header_parsed | START");
	int find = 0;
	ssize_t count = 0;
	switch (metadata->command){
		case GET:
			for(size_t i=0; i<vector_size(vector_file); i++){
				char* filename = vector_get(vector_file,i);
				if (strcmp(filename, metadata->filename) == 0){
					find = 1;
					metadata->type = OK;
					event.events = EPOLLOUT;
					epoll_ctl(efd, EPOLL_CTL_MOD, metadata->fd, &event);
					break;
				}
			}
			if (find != 1){
				LOG("GET: No such file");
				count = write_error_to_client(event, err_no_such_file);
				if (count == -1 || count == 0) {
					return count;
				}
			}
			break;
		case PUT:
			if (metadata->type == HEADER_PARSED){
				ssize_t size = get_message_size(metadata->fd);
				//printf("get_size:%ld\n",size);
				if (size == 0 || size == -1) return (int)size;
					
				metadata->size = size;
				metadata->remain = size;
				metadata->type = START;
			}
			if (metadata->type == START){	
				count = handle_put_read(event);
				if (count == -1 || count == 0) return count;
			}
			break;
		case DELETE:
			for(size_t i=0; i<vector_size(vector_file); i++){
				char* filename = vector_get(vector_file,i);
				if (strcmp(filename, metadata->filename) == 0){
					vector_erase(vector_file,i);
					find = 1;
					metadata->type = OK;
					char* file_path = malloc(strlen(server_path)+strlen(metadata->filename)+2);
					sprintf(file_path,"%s/%s", server_path, metadata->filename);
					LOG("Delete file:%s",file_path);
					unlink(file_path);
					free(file_path);
					break;
				}
			}
			if (find != 1){
				LOG("Delete: No such file");
				count = write_error_to_client(event, err_no_such_file);
				if (count == -1 || count == 0) {
					return count;
				}
			}
			break;
		case LIST:
			metadata->type = OK;
			break;
		default:
			LOG("Bad header");
			count = write_error_to_client(event, err_bad_request);
			if (count == -1 || count == 0) {
				return count;
			}
			break;
	}
	return 1;
}
int write_message_to_client(struct epoll_event event){
	state* metadata = event.data.ptr;	
	char* buffer;
	ssize_t count = 0;
	char* file_path;
	if (metadata->type == OK){
		LOG("OK");
		buffer = "OK\n";
		count = write(metadata->fd, buffer, strlen(buffer));
		if (count == -1 || count == 0) return count;
		metadata->type = RESPONSE;
	}
	if (metadata->type == RESPONSE){
		switch (metadata->command){
			case GET:
				if (metadata->size == 0){
					file_path = malloc(strlen(server_path)+strlen(metadata->filename)+2);
					sprintf(file_path,"%s/%s", server_path, metadata->filename);
					//LOG("GET:%s",file_path);
					size_t size = getFilesize(file_path);
					metadata->size = size;
					metadata->remain = size;
					write_message_size(size, metadata->fd);
				}
				handle_get_write(event);
				break;
			case LIST:
				LOG("LIST");		
				buffer = malloc(1024);
				buffer[0] = '\0';
				for(size_t i=0; i<vector_size(vector_file); i++){
					char* filename = vector_get(vector_file,i);
					strcat(buffer, filename);
					strcat(buffer, "\n");
				}
				LOG("Send list:%s",buffer);
				event.events = EPOLLOUT;
				epoll_ctl(efd, EPOLL_CTL_MOD, metadata->fd, &event);
				count = write_message_size(strlen(buffer), metadata->fd);
				if (count == -1) return count;
				if (strlen(buffer) != 0){		
					count = write(metadata->fd, buffer, strlen(buffer));
					if (count == -1 || count == 0) {
						free(buffer);
						return count;
					}
				}
				free(buffer);
				metadata->type = FINISH;				
				break;
			default:
				metadata->type = FINISH;
				break;
		}
	}
	return 1;
}
int handle_get_write(struct epoll_event event){
	state* metadata = event.data.ptr;
	ssize_t size = metadata->size;
	ssize_t remain = metadata->remain;
	ssize_t offset = size - remain;
	ssize_t len = remain;
	int first = 0;
	if (remain > MAX_CHUNK_SIZE) len = MAX_CHUNK_SIZE;
	if (remain == size) {
		first = 1;
	}
	char* file_path = malloc(strlen(server_path)+strlen(metadata->filename)+2);
    	sprintf(file_path,"%s/%s", server_path, metadata->filename);
   	LOG("GET:%s",file_path);
	LOG("Offset:%ld",offset);
	LOG("remain:%ld",metadata->remain);
    	FILE* input = fopen(file_path,"r+");
	fseek(input, offset, SEEK_SET);
	if (input == NULL) {
		perror("File can't find");
		metadata->type = FINISH;
		return -1;
	}
	free(file_path);
	char* buf = malloc(len+1);
	fread(buf, len, 1, input);
	fclose(input);
	ssize_t written_size = write_all_to_socket(metadata->fd, buf, len);
	if (written_size == 0 || written_size == -1) {
		free(buf);
		return written_size;
	}
        metadata->remain -= written_size;
	if (metadata->remain == 0) {
		metadata->type = FINISH;
	}
	free(buf);
	return 1;
}
int handle_put_read(struct epoll_event event){
	state* metadata = event.data.ptr;
	ssize_t size = metadata->size;
	ssize_t remain = metadata->remain;
	ssize_t offset = size - remain;
	ssize_t len = remain;
	int first = 0;
	if (remain > MAX_CHUNK_SIZE) len = MAX_CHUNK_SIZE;
	if (remain == size) {
		first = 1;
	}
	LOG("len:%zd",len);
	char* buf = malloc(MAX_CHUNK_SIZE);
	ssize_t receive_size = read_all_from_socket(metadata->fd, buf, len);
	LOG("size:%zd",receive_size);
	//printf("remain:%zd\n",remain);
	if (receive_size == -1 || receive_size == 0){	
		if (receive_size == 0 ) {
			print_too_little_data();
			receive_size = write_error_to_client(event, err_bad_file_size);
		}
		free(buf);
		return receive_size;
	}
	if (receive_size == len+1){
		print_received_too_much_data();
	}else if (receive_size < len){
		print_too_little_data();
		ssize_t count = write_error_to_client(event, err_bad_file_size);
		if(count == -1 || count == 0) {
			free(buf);
			return count;
		}
	}else{
		metadata->remain -= receive_size;
		remain -= receive_size;
		if (first){ 
			metadata->filecontent = malloc(metadata->size);
			memset(metadata->filecontent, 0, metadata->size);
		}		
		LOG("remain:%zd\n",remain);
		LOG("Offset:%ld", offset);
		memcpy(metadata->filecontent + offset, buf, receive_size);
		if (remain == 0){
			int find = 0;
			metadata->type = OK;
			event.events = EPOLLOUT;
			epoll_ctl(efd, EPOLL_CTL_MOD, metadata->fd, &event);
			for(size_t i=0; i<vector_size(vector_file); i++){
				char* filename = vector_get(vector_file,i);
				if (strcmp(filename, metadata->filename) == 0){
					find = 1;
					break;
				}
			}
			if (find == 0)
				vector_push_back(vector_file, metadata->filename);
			char* file_path = malloc(strlen(server_path)+strlen(metadata->filename)+2);
			sprintf(file_path,"%s/%s", server_path, metadata->filename);
			FILE* f = fopen(file_path,"w");
			fwrite(metadata->filecontent, metadata->size, 1, f);
			LOG("File:%s", file_path);
			free(file_path);
			//free(metadata->filecontent);
			//metadata->filecontent = NULL;
			fclose(f);
		}		
	}
	free(buf);
	return 1;
}
int write_error_to_client(struct epoll_event event, const char* error_msg){	
	state* metadata = event.data.ptr;
	event.events = EPOLLOUT;
	epoll_ctl(efd, EPOLL_CTL_MOD, metadata->fd, &event);
	char* buffer = malloc(6+strlen(error_msg)+2);
	sprintf(buffer,"ERROR\n%s\n",error_msg);
	ssize_t count = write(metadata->fd, buffer, strlen(buffer));
	if (count != -1 && count != 0) {
		metadata->type = FINISH;
	}
	free(buffer);
	return count;
}
int create_server(char *port) {
	
    int s;
    serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
 
    int optval = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
 
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    s = getaddrinfo(NULL, port, &hints, &result);
    if (s != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
            exit(1);
    }
    if (bind(serverSocket, result->ai_addr, result->ai_addrlen) != 0) {
        perror("bind()");
        exit(1);
    }
	freeaddrinfo(result);
    if (listen(serverSocket, 10) != 0) {
        perror("listen()");
        exit(1);
    }
    struct sockaddr_in *result_addr = (struct sockaddr_in *) result->ai_addr;
    printf("Listening on file descriptor %d, port %d\n", serverSocket, ntohs(result_addr->sin_port));
	server_path = malloc(7);
	strcpy(server_path,"XXXXXX");
	server_path = mkdtemp(server_path);
	print_temp_directory(server_path);
	return 0;
}
static int make_socket_non_blocking(int socket)
{
	int flags, s;
	flags = fcntl(socket, F_GETFL,0);
	if(flags == -1)
	{
		perror("fcntl");
		return-1;
	}

	flags|= O_NONBLOCK;
	s =fcntl(socket, F_SETFL, flags);
	if(s ==-1)
	{
		perror("fcntl");
		return-1;
	}
	return 0;
}
size_t getFilesize(const char* filename) {
    struct stat st;
    if(stat(filename, &st) != 0) {
        return 0;
    }
    return st.st_size;   
}
