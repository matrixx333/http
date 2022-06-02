#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>	//socket address structures, byte-ordering functions
#include <arpa/inet.h> //address conversion functions
#include <sys/socket.h> //socket(), connect(), send(), etc...
#include <sys/types.h>
#include <netdb.h> //addrinfo structure, gai_strerror()
#include <unistd.h>
#include <errno.h>

#define SIZE 10100	

ssize_t readn(int fd, void *vptr, size_t n);

int main(int argc, char **argv) {
	struct addrinfo hints, *res = NULL;
	int sockfd = 0; //socket() return value
	int status = 0; //getaddrinfo() return value
	int value = 0; //connect() return value
	ssize_t	sent = 0; //send() return value
	ssize_t received = 0; //recv() return value
	size_t msg_len = 0; //length of http GET method
	char *buf = NULL; //recv() buffer
	char request[100] = {0}; //http GET method

	
	sprintf(request, "GET /index.html HTTP/1.1\n"
			 "Host: %s\n\r\n\r", argv[1]);
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
	printf("%s\n", request);

	if((status = getaddrinfo(argv[1], "80", &hints, &res)) > 0) {
		fprintf(stderr, "%s\n", gai_strerror(status));
		exit(EXIT_FAILURE);
	}

	if((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}


	value = connect(sockfd, res->ai_addr, res->ai_addrlen);
	if (value == -1) {
		close(sockfd);	
		perror("connect");
		exit(EXIT_FAILURE);
	}
	else 
		printf("connect succeeded\n");

	freeaddrinfo(res);

	msg_len = strlen(request);
	printf("msg_len: %d-bytes\n", msg_len);
	
	if((sent = send(sockfd, request, msg_len, 0)) == -1) {
		perror("send");
		exit(EXIT_FAILURE);
	}

	printf("bytes sent: %d\n\n", sent);
	
	buf = malloc(SIZE);
	if(buf == NULL) {
		fprintf(stderr, "malloc() failed.\n");
		exit(EXIT_FAILURE);
	}

	memset(buf, 0, SIZE);
	
	if((received = readn(sockfd, buf, SIZE)) == -1) {
		perror("readn");
		exit(EXIT_FAILURE);
	}

	printf("%s\n", buf);	
	printf("\nreceived: %d-bytes\n", received);
	
	close(sockfd);
	free(buf);

	return 0;
}

ssize_t readn(int fd, void *vptr, size_t n) {
	size_t nleft;
	ssize_t nread;
	char *ptr;

	ptr = vptr;
	nleft = n;
	while(nleft > 0) {
		if((nread = read(fd, ptr, nleft)) < 0) {
			if(errno == EINTR)
				nread = 0;
			else 
				return (-1);
		}
		else if(nread == 0)
			break;

		nleft -= nread;
		ptr += nread;
	}
	return (n - nleft);
}


