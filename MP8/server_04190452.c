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
#include "common.h"
#define MAXEVENTS 100
struct _state{
	int fd;
	verb command;
	state_type type;
	size_t size;
};
typedef struct _state state;
static volatile int serverSocket;
static struct addrinfo hints, *result;
int create_server(char *port);
static volatile int endSession;
static int make_socket_non_blocking(int socket);
int read_message_from_client(int client_fd, state* metadata, char* command, char* filename);
void close_server() {
	endSession = 1;
    if (shutdown(serverSocket, SHUT_RDWR) != 0) {
        perror("shutdown():");
    }
    close(serverSocket);
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
	event.events= EPOLLIN;
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
		for(int i=0; i<num_ready; i++)
		{
			if((events[i].events & EPOLLERR)||
					(events[i].events & EPOLLHUP)||
					(!(events[i].events & EPOLLIN)))
			{
				/* An error has occured on this fd, or the socket is not
				   ready for reading (why were we notified then?) */
				fprintf(stderr,"epoll error\n");
				close(events[i].data.fd);
				continue;
			}

			else if(serverSocket == events[i].data.ptr->fd)
			{
				/* We have a notification on the listening socket, which
				   means one or more incoming connections. */
				while(1)
				{
					int client_fd;
					struct sockaddr in_addr;
					socklen_t in_len;
					char hbuf[NI_MAXHOST],sbuf[NI_MAXSERV];
					
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
					printf("p:%p\n",event.data.ptr);
					//printf("fdp:%p\n",&event.data.fd);
					//printf("test:%d\n",event.data.fd);
					
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
				int done = 0;
				//while(1)
				//{
					ssize_t count;
					char* command = malloc(10);
					char* filename = malloc(256);
					state* metadata = events[i].data.ptr;
					
					count = read_message_from_client(events[i].data.fd, metadata, command, filename);
					done = 1;
					//count = read(events[i].data.fd, buf, sizeof(buf));
					/*if(count == -1)
					{ 
					
						if(errno!= EAGAIN)
						{
							perror("read");
							done=1;
						}
						//break;
					}
					else if(count ==0)
					{
						done=1;
						//break;
					}
					else 
					{
						
					}
					// Write the buffer to standard output */
					s = write(events[i].data.fd ,"OK\ntest.txt", 12);
					if(s ==-1)
					{
						//perror("write");
					}
				//}
				if(done)
				{
					//printf("Closed connection on descriptor %d\n",events[i].data.fd);
					/* Closing the descriptor will make epoll remove it
					   from the set of descriptors which are monitored. */
					close(events[i].data.fd);
				}
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
	
	if (create_server(argv[1]) != 0)  return 1;
	
	if (start_server() != 0) {
		close_server();
		return 1;
	}
	return 0;
	
}
int read_message_from_client(int client_fd, state* metadata, char* command, char* filename){
	ssize_t count = 0;
	if (metadata->type == INITIAL){
	//	LOG("Initial");
		char buffer[280];
		count = read_until_newline(client_fd, buffer);
		//printf("count:%ld\n",count);
		if (count == 0) return 0;
		if (count == -1) {
			return -1;
		}
		int index = 0;
		int filename_index = 0;
		//char* command = malloc(10);
		//char* filename = malloc(256);	
		int end = 0;
		while (index < count){
			if (index < 10 && buffer[index] != ' ' && !end){
				command[index] = *(buffer+index);
			}
			else if (buffer[index] != ' '){
				filename[filename_index] = *(buffer+index);
				filename_index ++;
			}
			if (buffer[index] == ' ' && end == 0){
				command[index] = '\0';
				end = 1;
			}
			index ++;
		}
		filename[filename_index] = '\0';
		return count;
		if (strcmp(command, "LIST") == 0) metadata->command = LIST;
		else if (strcmp(command, "PUT") == 0) metadata->command = PUT;
		else if (strcmp(command, "GET") == 0) metadata->command = GET;
		else if (strcmp(command, "DELETE") == 0) metadata->command = DELETE;
		else metadata->command = V_UNKNOWN;
		metadata->type = HEADER_PARSED;
	}
	if (metadata->type == HEADER_PARSED){
		LOG("Header_parsed");
		switch (metadata->command){
			case GET:
			case PUT:
			case DELETE:
			case LIST:
			default:
				break;
		}
	}
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