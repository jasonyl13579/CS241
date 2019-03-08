/**
* Chatroom Lab
* CS 241 - Spring 2018
*/

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include "utils.h"

#define MAX_CLIENTS 8

void *process_client(void *p);
void cleanup();
static volatile int serverSocket;
static volatile int endSession;
static volatile int clientsCount;
static volatile int clients[MAX_CLIENTS];
static struct addrinfo hints, *result;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t threads[MAX_CLIENTS];
static bool have_connection[MAX_CLIENTS];
/**
 * Signal handler for SIGINT.
 * Used to set flag to end server.
 */
void close_server() {
	
	cleanup();
	freeaddrinfo(result);
    endSession = 1;
    // add any additional flags here you want.
}

/**
 * Cleanup function called in main after `run_server` exits.
 * Server ending clean up (such as shutting down clients) should be handled
 * here.
 */
void cleanup() {
    if (shutdown(serverSocket, SHUT_RDWR) != 0) {
        perror("shutdown():");
    }
    close(serverSocket);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1) {
            if (shutdown(clients[i], SHUT_RDWR) != 0) {
                perror("shutdown(): ");
            }
            close(clients[i]);
        }
    }
}

/**
 * Sets up a server connection.
 * Does not accept more than MAX_CLIENTS connections.  If more than MAX_CLIENTS
 * clients attempts to connects, simply shuts down
 * the new client and continues accepting.
 * Per client, a thread should be created and 'process_client' should handle
 * that client.clientsCount
 * Makes use of 'endSession', 'clientsCount', 'client', and 'mutex'.
 *
 * port - port server will run on.
 *
 * If any networking call fails, the appropriate error is printed and the
 * function calls exit(1):
 *    - fprinf to stderr for getaddrinfo
 *    - perror() for any other call
 */
void run_server(char *port) {
    /*QUESTION 1*/   
    /*QUESTION 2*/
    /*QUESTION 3*/
	for (int i=0;i<MAX_CLIENTS;i++) clients[i] = -1; 
    clientsCount = 0;
    int s;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    /*QUESTION 8*/
    int optval = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    /*QUESTION 4*/
    /*QUESTION 5*/
    /*QUESTION 6*/
   // struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    s = getaddrinfo(NULL, port, &hints, &result);
    if (s != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
            exit(1);
    }
    /*QUESTION 9*/
    if (bind(serverSocket, result->ai_addr, result->ai_addrlen) != 0) {
        perror("bind()");
        exit(1);
    }
    /*QUESTION 10*/
    if (listen(serverSocket, 10) != 0) {
        perror("listen()");
        exit(1);
    }
	endSession = 0;
    struct sockaddr_in *result_addr = (struct sockaddr_in *) result->ai_addr;
    printf("Listening on file descriptor %d, port %d\n", serverSocket, ntohs(result_addr->sin_port));
	
	int clientsIndex[MAX_CLIENTS];
	for (int i=0;i<MAX_CLIENTS;i++) clientsIndex[i] = i;
	for (int i=0;i<MAX_CLIENTS;i++) have_connection[i] = false;
	while( !endSession ){
		printf("Waiting for connection...\n");
		int client_fd = accept(serverSocket, NULL, NULL);
		int index = 0;
		
		for (index = 0 ;index < MAX_CLIENTS ;index++) {
			if(!have_connection[index])break;
		}	
		fprintf(stderr,"index:%d\n",index);
		if (index >= MAX_CLIENTS) {
			close(client_fd);
			shutdown(client_fd , SHUT_RDWR);
			fprintf(stderr,"Reach limit clients\n");
			continue;
		}
		fprintf(stderr,"Connection made: client_fd=%d\n", client_fd);
		pthread_mutex_lock(&mutex);
		clients[index] = client_fd;
		clientsCount ++;
		have_connection[index] = true;
		pthread_mutex_unlock(&mutex);
		pthread_create(&threads[index], NULL, process_client, (void*)&clientsIndex[index]);		
		printf("count:%d\n",clientsCount);
		//printf("p:%p\n",clientsIndex);
		
		
	}
	puts("end");
    //clientsCount ++;
	//pthread_join(thread, NULL);
}

/**
 * Broadcasts the message to all connected clients.
 *
 * message  - the message to send to all clients.
 * size     - length in bytes of message to send.
 */
void write_to_clients(const char *message, size_t size) {
    pthread_mutex_lock(&mutex);
	printf("msg:%s\n",message);
	fflush(stdout);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1) {
			//printf("test\n");
            ssize_t retval = write_message_size(size, clients[i]);
            if (retval > 0) {
                retval = write_all_to_socket(clients[i], message, size);
            }
            if (retval == -1) {
                perror("write(): ");
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}

/**
 * Handles the reading to and writing from clients.
 *
 * p  - (void*)intptr_t index where clients[(intptr_t)p] is the file descriptor
 * for this client.
 *
 * Return value not used.
 */
void *process_client(void *p) {
    pthread_detach(pthread_self());
    int clientId = *(int*)p;
    ssize_t retval = 1;
    char *buffer = NULL;
    while (retval > 0 && endSession == 0) {
        retval = get_message_size(clients[clientId]);
		//printf("ret:%zd\n",retval);
        if (retval > 0) {
            buffer = calloc(1, retval);
            retval = read_all_from_socket(clients[clientId], buffer, retval);
			//printf("msg:%s,retval:%zd\n",buffer,retval);
			//fflush(stdout);
        }
        if (retval > 0)
            write_to_clients(buffer, retval);

        free(buffer);
        buffer = NULL;
    }

    printf("User %d left\n", (int)clientId);
    close(clients[clientId]);
	have_connection[clientId] = false;
    pthread_mutex_lock(&mutex);
    clients[clientId] = -1;
    clientsCount--;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "%s <port>\n", argv[0]);
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
    run_server(argv[1]);
	//cleanup();
    pthread_exit(NULL);
}
