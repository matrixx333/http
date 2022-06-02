#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "url.h"

void *set_mem(size_t size){
	void *ret = NULL;

	ret = malloc(size);
	if(ret == NULL){
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	return ret;
}

void init_struct_elem(char **ptr, char *url, size_t size){
	*ptr = set_mem(size+1);
	strncpy(*ptr, url, size);
}

int checkurl(char *url){
	int i = 0;	
	
	if(url == NULL)
		return 1;

	// perform error checking on the url
	while(url[i] != '\0'){
		if(url[i] == ':'){
			if(url[i+1] == '\0')
				return 1;
			if(isalpha(url[i+1]))
				return 1;
		}
		i++;
	}

	return 0;
}

int parse_url(struct web_addr **addr, char *url){
	struct web_addr *ptr = NULL;
	int i = 0;
	char flag = NONE;

	if(checkurl(url))
		return 1;

	ptr = set_mem(sizeof(struct web_addr));
	
	ptr->protocol = NULL;
	ptr->host = NULL;
	ptr->port = NULL;
	ptr->abs_path = NULL;
	ptr->query = NULL;
	ptr->fragment = NULL;

	while(1){
		if(url[i] == '\0'){
			if((flag == PROTOCOL) || (flag == NONE)){
				init_struct_elem(&ptr->host, url, i);
				break;
			}
			if(flag == PORT){
				init_struct_elem(&ptr->port, url, i);
				break;
			}
			if(flag == ABS_PATH){
				init_struct_elem(&ptr->abs_path, url, i);
				break;
			}
			if(flag == QUERY){
				init_struct_elem(&ptr->query, url, i);
				break;
			}
			if(flag == FRAGMENT){
				init_struct_elem(&ptr->fragment, url, i);
				break;
			}
		}
		if(url[i] == ':'){
			// is protocol
			if(url[i+1] == '/'){
				init_struct_elem(&ptr->protocol, url, i);
				flag = PROTOCOL;
				url += i+3;
				i = 0;
			}
			// is port
			else{
				init_struct_elem(&ptr->host, url, i);
				url += i+1;
				flag = PORT;
				i = 0;
			}
		}
		if(url[i] == '/'){
			if(flag == PORT){
				init_struct_elem(&ptr->port, url, i);
				url += i;
				i = 0;
			}
			if((flag == NONE) || (flag == PROTOCOL)){
				init_struct_elem(&ptr->host, url, i);
				url += i;
				i = 0;
			}
			flag = ABS_PATH;	
		}
		if(url[i] == '?'){
			init_struct_elem(&ptr->abs_path, url, i);
			url += i+1;
			flag = QUERY;
			i = 0;
		}
		if(url[i] == '#'){
			init_struct_elem(&ptr->query, url, i);
			url += i+1;
			flag = FRAGMENT;
			i = 0;
		}
		i++;
	}

	*addr = ptr;

	return 0;
}

void set_defaults(struct web_addr *ptr){
	if(ptr->protocol == NULL)
		init_struct_elem(&ptr->protocol, "http", strlen("http"));
	if(ptr->port == NULL)
		init_struct_elem(&ptr->port, "80", strlen("80"));
	if(ptr->abs_path == NULL)
		init_struct_elem(&ptr->abs_path, "/", strlen("/"));
}

void free_mem(struct web_addr *ptr){
	if(ptr->protocol != NULL)
		free(ptr->protocol);
	if(ptr->host != NULL)
		free(ptr->host);
	if(ptr->port != NULL)
		free(ptr->port);
	if(ptr->abs_path != NULL)
		free(ptr->abs_path);
	if(ptr->query != NULL)
		free(ptr->query);
	if(ptr->fragment != NULL)
		free(ptr->fragment);
}
