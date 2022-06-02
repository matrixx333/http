#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "connect.h"

void setupConnect(struct _structptr *ptr, char *node, char *service){
	initStruct(ptr, node, service);
	
	if((ptr->error = getaddrinfo(node, service, ptr->hints, 
							&ptr->res)) != 0){
		perror(gai_strerror(ptr->error));
		exit(EXIT_FAILURE);
	}
	
	connectServer(ptr);	
	convertNtop(ptr);
}

void initStruct(struct _structptr *ptr, char *node, char *service){
	struct addrinfo *hints = NULL;
	
	ptr->hints = heap(sizeof(struct addrinfo));
	ptr->sa = heap(sizeof(struct sockaddr_in));
	ptr->ip_addr = heap(INET_ADDRSTRLEN);
	ptr->node = node;
	ptr->service = service; 
	ptr->sock = 0;
	ptr->error = 0;
	
	hints = ptr->hints;
	
	memset(hints, 0, sizeof(struct addrinfo));
	hints->ai_family = AF_UNSPEC;
	hints->ai_socktype = SOCK_STREAM;
	hints->ai_protocol = 0;
}

void *heap(size_t size){
	void *ptr = NULL;
	
	if((ptr = malloc(size)) == NULL){
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	return ptr;
}

void connectServer(struct _structptr *ptr){
	struct addrinfo *res = ptr->res;
	
	if((ptr->sock = socket(res->ai_family, res->ai_socktype, 
							res->ai_protocol)) < 0){
		perror("socket");
		exit(EXIT_FAILURE);
	}
	
	if((connect(ptr->sock, (const struct sockaddr *)res->ai_addr, 
							(socklen_t)res->ai_addrlen)) != 0){
		perror("connect");
		exit(EXIT_FAILURE);
	}	
}

void convertNtop(struct _structptr *ptr){
	struct addrinfo *res = NULL;
	struct sockaddr_in *sa = NULL;
	
	res = ptr->res;
	sa = (struct sockaddr_in *)res->ai_addr;
	ptr->sa = sa;
	
	inet_ntop(res->ai_family, &(sa->sin_addr), ptr->ip_addr, 
							(socklen_t)res->ai_addrlen);	
}

void freePtr(struct _structptr *ptr){
	free(ptr->hints);
	free(ptr->res);
	free(ptr->sa);
	free(ptr->ip_addr);
	
	ptr->hints = NULL;
	ptr->res = NULL;
	ptr->sa = NULL;
	ptr->ip_addr = NULL;
}
