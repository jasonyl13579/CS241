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
#include "common.h"
#include "format.h"
#define MAXEVENTS 100
#define MAX_CHUNK_SIZE 1023
struct _state{
	int fd;
	verb command;
	state_type type;
	char* filename;
	size_t size;
	size_t remain;
	char* filecontent;
};
typedef struct _state state;
static volatile int serverSocket;
static struct addrinfo hints, *result;
static char* server_path;
static vector* vector_file;
int create_server(char *port);
static volatile int endSession;

static int make_socket_non_blocking(int socket);
int read_message_from_client(state* metadata);
int handle_initial_state(state* metadata);
int handle_parsed_start_state(state* metadata);
int write_message_to_client(state* metadata);
int handle_put_read(state* metadata);
int handle_get_write(state* metadata);

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
	int efd, s;
	if((efd = epoll_create1(0)) == -1){
		perror("epoll_create");
		exit(1);
	}
	//event.data.fd = serverSocket;
	event.events= EPOLLIN ;
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
		if (num_ready == 0) puts("11");
		for(int i=0; i<num_ready; i++)
		{
			state* metadata = events[i].data.ptr;
			if((events[i].events & EPOLLERR)||
					(events[i].events & EPOLLHUP)||
					(!(events[i].events & EPOLLIN)))
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
					puts("....");
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
					count = read_message_from_client(metadata);
					//count = read(events[i].data.fd, buf, sizeof(buf));
					if(count == -1)
					{ 
					
						if(errno != EAGAIN || errno != EWOULDBLOCK)
						{
							metadata->type = FINISH;
							
						}
						if(errno == EPIPE){
							metadata->type = FINISH;	
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
						if (metadata->filename) free(metadata->filename);
						if (metadata->f != NULL) fclose(metadata->f);
						free(metadata);
						//break;
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
int read_message_from_client(state* metadata){
	if (metadata->type == INITIAL){
		int r = handle_initial_state(metadata);
		if (r == -1 || r == 0) return r;
	}
	if (metadata->type == HEADER_PARSED || metadata->type == START){
		int r = handle_parsed_start_state(metadata);
		if (r == -1 || r == 0) return r;
	}
		
	if(metadata->type == OK)
		write_message_to_client(metadata);
	return 1;
}
int handle_initial_state(state* metadata){
	ssize_t count = 0;
	int client_fd = metadata->fd;
	//LOG("Initial");
	char buffer[280];
	count = read_until_newline(client_fd, buffer);
	if (count != 0)
	//printf("count:%ld\n",count);
	if (count == 0) return 0;
	if (count == -1) {
		return -1;
	}
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
int handle_parsed_start_state(state* metadata){
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
					break;
				}
			}
			if (find != 1){
				LOG("GET: No such file");
				char* buf = malloc(6+strlen(err_no_such_file)+2);
		sprintf(buf,"ERROR\n%s\n",err_no_such_file);
				//LOG("%s",buf);
				count = write(metadata->fd, buf, strlen(buf));
				if (count == -1 || count == 0) {
					free(buf);
					return count;
				}
				metadata->type = FINISH;
				free(buf);
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
				count = handle_put_read(metadata);
				if (count == -1 || count == 0) return count;
				/*ssize_t size = metadata->size;
				//printf("get_size2:%ld\n",size);
				char* buf = malloc(size+1);				
				ssize_t receive_size = read_all_from_socket(metadata->fd, buf, size);
				//printf("size:%zd\n",receive_size);
				if (receive_size == -1 || receive_size == 0){
					free(buf);
					return receive_size;
				}
				buf[size] = '\0';
				if (receive_size == size+1){
					print_received_too_much_data();
				}else if (receive_size < size){
					print_too_little_data();
				}else{
					char* file_path = malloc(strlen(server_path)+strlen(metadata->filename)+2);
					sprintf(file_path,"%s/%s", server_path, metadata->filename);
					FILE* f = fopen(file_path,"w");				
					LOG("File:%s",file_path);
					for(size_t i=0; i<vector_size(vector_file); i++){
						char* filename = vector_get(vector_file,i);
						if (strcmp(filename, metadata->filename) == 0){
							find = 1;
							break;
						}
					}
					if (find == 0)
						vector_push_back(vector_file, metadata->filename);
					fprintf(f,"%s",buf);
					fclose(f);
					free(file_path);
					metadata->type = OK;
				}
				free(buf);*/
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
				char* buf = malloc(6+strlen(err_no_such_file)+2);
				sprintf(buf,"ERROR\n%s\n",err_no_such_file);
				//LOG("%s",buf);
				count = write(metadata->fd, buf, strlen(buf));
				if (count == -1 || count == 0) {
					free(buf);
					return count;
				}
				metadata->type = FINISH;
				free(buf);
			}
			break;
		case LIST:
			metadata->type = OK;
			break;
		default:
			LOG("Bad header");
			char* buf = malloc(6+strlen(err_bad_request)+2);
			sprintf(buf,"ERROR\n%s\n",err_bad_request);
			//LOG("%s",buf);
			count = write(metadata->fd, buf, strlen(buf));
			if (count == -1 || count == 0){
				free(buf);
				return count;
			}
			metadata->type = FINISH;
			free(buf);
			break;
	}
	return 1;
}
int write_message_to_client(state* metadata){
	char* buffer;
	ssize_t count = 0;
	char* file_path;
	if (metadata->type == OK){
		LOG("OK");
		buffer = "OK\n";
		count = write(metadata->fd, buffer, strlen(buffer));
		if (count == -1 || count == 0) return count;
	}
	switch (metadata->command){
		case GET:
			if (metadata->size == 0){
				file_path = malloc(strlen(server_path)+strlen(metadata->filename)+2);
				sprintf(file_path,"%s/%s", server_path, metadata->filename);
				LOG("GET:%s",file_path);
				FILE* input = fopen(file_path,"r+");
				if (input == NULL) {
					perror("File can't find");
					metadata->type = FINISH;
					return -1;
				}
				fseek(input, 0L, SEEK_END);
				int64_t size = ftell(input);
				fseek(input, 0L, SEEK_SET);
				metadata->size = size;
				metadata->remain = size;
				write_message_size(size, metadata->fd);
			}
			handle_get_write(metadata);
			/*char *file_bytestream = malloc(size+1);
			if (fread(file_bytestream, 1, size, input) == 0){
				fclose(input);
				return -1;
			}
			file_bytestream[size] = '\0';
			fclose(input);
			count = write(metadata->fd, file_bytestream, strlen(file_bytestream));
			free(file_bytestream);
			free(file_path);
			if (count == -1 || count == 0) return count;*/
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
			break;
		default:
			break;
	}
	metadata->type = FINISH;
	return 1;
}
int handle_get_write(state* metadata){
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
        FILE* input = fopen(file_path,"r+");
	fseek(input,SEEK_SET,offset);
        if (input == NULL) {
        	perror("File can't find");
              	metadata->type = FINISH;
         	return -1;
      	}
	free(file_path);
	char* buf = malloc(len+1);
	fread(buf, len, 1, input);
        ssize_t written_size = write_all_to_socket(metadata->fd, buf, len);
	if (written_size == 0 || written_size == -1) {
		free(buf);
		return written_size;
	}
        metadata->remain -= written_size;
	LOG("remain:%ld",metadata->remain);
	if (metadata->remain == 0) {
		metadata->type = FINISH;
	}
	free(buf);
	return 1;
}
int handle_put_read(state* metadata){
	ssize_t size = metadata->size;
	ssize_t remain = metadata->remain;
	ssize_t offset = size - remain;
	ssize_t len = remain;
	int first = 0;
	if (remain > MAX_CHUNK_SIZE) len = MAX_CHUNK_SIZE;
	if (remain == size) {
		first = 1;
	}
	printf("len:%zd\n",len);
	char* buf = malloc(MAX_CHUNK_SIZE + 1);
	ssize_t receive_size = read_all_from_socket(metadata->fd, buf, len);
	printf("size:%zd\n",receive_size);
	//printf("remain:%zd\n",remain);
	if (receive_size == -1 || receive_size == 0){
		free(buf);
		return receive_size;
	}
	if (receive_size == len+1){
		print_received_too_much_data();
	}else if (receive_size < len){
		print_too_little_data();
		char* buffer = malloc(6+strlen(err_bad_file_size)+2);
                sprintf(buffer,"ERROR\n%s\n",err_bad_file_size);
                                //LOG("%s",buf);
                ssize_t count = write(metadata->fd, buffer, strlen(buffer));
                if (count == -1 || count == 0) {
                	free(buffer);
		     	free(buf);
                     	return count;
                }
                metadata->type = FINISH;
                free(buffer);
	}else{
		metadata->remain -= receive_size;
		remain -= receive_size;
		if (first){ 
			char* file_path = malloc(strlen(server_path)+strlen(metadata->filename)+2);
			sprintf(file_path,"%s/%s", server_path, metadata->filename);
			metadata->f = fopen(file_path,"w");
			LOG("File:%s, offset:%ld", file_path, offset);
			free(file_path);
		}		
		LOG("remain:%zd\n",remain);
		fwrite(buf,receive_size,1,metadata->f);
		if (remain == 0){
			int find = 0;
			metadata->type = OK;
			for(size_t i=0; i<vector_size(vector_file); i++){
				char* filename = vector_get(vector_file,i);
				if (strcmp(filename, metadata->filename) == 0){
					find = 1;
					break;
				}
			}
			if (find == 0)
				vector_push_back(vector_file, metadata->filename);
			fclose(metadata->f);
			metadata->f = NULL;
		}		
	}
	free(buf);
	return 1;
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
