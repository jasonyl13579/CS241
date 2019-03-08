/**
* Networking Lab
* CS 241 - Spring 2018
*/

#include "common.h"
#include "format.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
char **parse_args(int argc, char **argv);
verb check_args(char **args);
int connect_to_server(const char *host, const char *port);
void send_message_to_server(verb command, char **args);
void read_message_from_server(verb command, char **args);
size_t read_response_from_server();
static volatile int sock_fd;
static struct addrinfo hints, *result;

int main(int argc, char **argv) {
    // Good luck!
	char** args = parse_args(argc, argv);
	verb method = check_args(args);
	char* host = args[0];
	char* port = args[1];
	sock_fd = connect_to_server(host, port);
	//create_message(method, args);
	send_message_to_server(method, args);
	shutdown(sock_fd, SHUT_RDWR);
	close(sock_fd);
	free(args);
	freeaddrinfo(result);
	return 0;

}
int connect_to_server(const char *host, const char *port) {
	int s;
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);	
	
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET; /* IPv4 only */
	hints.ai_socktype = SOCK_STREAM; /* TCP */
	s = getaddrinfo(host, port, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(1);
	}
	if(connect(sock_fd, result->ai_addr, result->ai_addrlen) == -1){
		perror("connect");
		exit(1);
	}
	puts("connect");
    return sock_fd;
}
void send_message_to_server(verb command, char **args) {
	char *buffer = NULL;
	switch (command){
		case GET:
			//LOG("GET");
			buffer = malloc(6+strlen(args[3]));
			sprintf(buffer,"GET %s\n",args[3]);
			LOG("%s",buffer);
			write_all_to_socket(sock_fd, buffer, strlen(buffer));
			shutdown(sock_fd, SHUT_WR);
			free(buffer);
			break;
		case PUT:
			LOG("PUT");
			
			FILE* input = fopen(args[4],"r+");
			if (input == 0) {
				print_error_message("No such file");
				return;
			}
			buffer = malloc(6+strlen(args[3]));
			sprintf(buffer,"PUT %s\n",args[3]);
			LOG("%s",buffer);
			write_all_to_socket(sock_fd, buffer, strlen(buffer));
			free(buffer);
			
			fseek(input, 0L, SEEK_END);
			int64_t size = ftell(input);
			fseek(input, 0L, SEEK_SET);
			LOG("Filesize:%ld",size);
			write_message_size(size, sock_fd);
			char *file_bytestream = malloc(size+1);
			if (fread(file_bytestream, 1, size, input) == 0){
				fclose(input);
				return;
			}
			file_bytestream[size] = '\0';
			//printf("%s\n",file_bytestream);
			fclose(input);
			LOG("Sent %zd bytes of file",size);
			write_all_to_socket(sock_fd, file_bytestream, size);
			shutdown(sock_fd, SHUT_WR);
			free(file_bytestream);
			break;
		case LIST:
			//LOG("LIST");
			buffer = "LIST\n";
			write_all_to_socket(sock_fd, buffer, strlen(buffer));
			shutdown(sock_fd, SHUT_WR);
			break;
		case DELETE:
			//LOG("DELETE");
			buffer = malloc(9+strlen(args[3]));
			sprintf(buffer,"DELETE %s\n",args[3]);
			LOG("%s",buffer);
			write_all_to_socket(sock_fd, buffer, strlen(buffer));
			shutdown(sock_fd, SHUT_WR);
			free(buffer);
			break;
		default:
			break;
	}
	read_message_from_server(command, args);
   /* int name_len = strlen(name);
    int msg_len = strlen(message);
    char *msg = calloc(1, msg_len + name_len + 4);
    sprintf(msg, "%s: %s", name, message);*/
}
void read_message_from_server(verb command, char **args) {
	ssize_t size;
	//sleep(5);	
	size = read_response_from_server();
	ssize_t len = 0;
	ssize_t remain = 0;
	ssize_t receive_size = 0;
	if (size == 0) return;
	char *buffer = malloc(MAX_CHUNK_SIZE);
	switch (command){		
		case GET:
			//LOG("GET_RESPONSE");			
			size = get_message_size(sock_fd);
			len = size;
			remain = size;
			LOG("Expecting %zd bytes from server",size);
			FILE* out = fopen(args[4],"w+");
			while(1){
				if (remain > 0){				
					if (remain > MAX_CHUNK_SIZE) len = MAX_CHUNK_SIZE;
					receive_size = read_all_from_socket(sock_fd, buffer, len); // extra one bit to test received_too_much_data
					//LOG("size:%zd",receive_size);
					if (receive_size > len){
						print_received_too_much_data();
					}else if (receive_size == 0){
						print_too_little_data();
						break;
					}else if (receive_size == -1 && errno != EAGAIN){
						print_too_little_data();
						break;
					}else{
						//LOG("remain:%ld\n", remain);
						remain -= receive_size;
						fwrite(buffer, receive_size, 1, out);
					}						
				}else break;
			}
			if (read_all_from_socket(sock_fd, buffer, 1) !=0)  print_received_too_much_data();
			LOG("Received %zd bytes from server",size - remain);	
			
			fclose(out);
			break;
		case PUT:
			//LOG("PUT_RESPONSE");
			break;
		case LIST:
			//LOG("LIST_RESPONSE");
			size = get_message_size(sock_fd);
			len = size;
			remain = size;
			LOG("Expecting %zd bytes from server",size);
			
			while(remain > 0){			
				if (remain > MAX_CHUNK_SIZE) 
					len = MAX_CHUNK_SIZE;
				receive_size = read_all_from_socket(sock_fd, buffer, len); // extra one bit to test received_too_much_data
					//LOG("size:%zd",receive_size);
				if (receive_size > len){
					print_received_too_much_data();
				}else if (receive_size == 0){
					print_too_little_data();
					break;
				}else if (receive_size == -1 && errno != EAGAIN){
					print_too_little_data();
					break;
				}else{
					//FILE* out = fopen(args[4],"w+");
					//LOG("remain:%ld\n", remain);
					remain -= receive_size;
					write_all_to_socket(1, buffer, receive_size);
				}						
			}
			if (read_all_from_socket(sock_fd, buffer, 1) != 0)  print_received_too_much_data();
			LOG("Received %zd bytes from server",size - remain);	
			break;
		case DELETE:
			//LOG("DELETE_RESPONSE");
			break;
		default:
			break;
	}
	free(buffer);
}
size_t read_response_from_server()
{
	char *buffer = malloc(100);
	size_t count = read_until_newline(sock_fd, buffer);
	printf("Control msg size:%zd\n",count);
	buffer[count] = '\0';
	printf("response:%s\n",buffer);
	if (strcmp(buffer,"OK\n") == 0)
		printf("%s",buffer);
	else if (strcmp(buffer,"ERROR\n") == 0){
		count = read_until_newline(sock_fd, buffer);
		printf("%s",buffer);
		free(buffer);
		return 0;
	}
	else print_invalid_response();
	free(buffer);
	return count;
}
/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}
