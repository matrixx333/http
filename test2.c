#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define MAX		100

#define NONE		0x0
#define FRAGMENT	0x1
#define QUERY		0x2
#define ABS_PATH	0x4
#define PORT		0x8
#define HOST		0x10
#define PROTOCOL 	0x20

struct web_addr {
	char *protocol;
	char *host;
	char *port;
	char *abs_path;
	char *query;
	char *fragment;
};

void *set_mem(size_t size);
void init_protocol_mem(struct web_addr *ptr, char *url, size_t size);
void init_host_mem(struct web_addr *ptr, char *url, size_t size);
void init_port_mem(struct web_addr *ptr, char *url, size_t size);
void init_abs_path_mem(struct web_addr *ptr, char *url, size_t size);
void init_query_mem(struct web_addr *ptr, char *url, size_t size);
void init_frag_mem(struct web_addr *ptr, char *url, size_t size);
int parse_url(struct web_addr **addr, char *url);
void set_defaults(struct web_addr *ptr);
void free_mem(struct web_addr *ptr);

int main(int argc, char **argv){
	struct web_addr	*addr = NULL;
	char url[MAX] = {0};
	int ret = 0;
	int i = 0;
	FILE *fin = NULL, *fout = NULL;
	char *filename = "testcase.txt";
	char *output = "results.txt";

	fin = fopen(filename, "r");
	if(fin == NULL){
		printf("Error: unable to open %s\n", filename);
		exit(EXIT_FAILURE);
	}

	fout = fopen(output, "w");
	if(fout == NULL){
		printf("Error: unable to write to %s\n", output);
		exit(EXIT_FAILURE);
	}
	
	while(fscanf(fin, "%s", url) != EOF){
		ret = parse_url(&addr, url);
		if(!ret){
			printf("Error: check your url\n");
			exit(EXIT_FAILURE);
		}

		set_defaults(addr);

		fprintf(fout,	"Test %d\n"
				"protocol: %s\n"
				"host: %s\n"
				"port: %s\n" 
				"abs_path: %s\n"
				"query: %s\n"
				"fragment: %s\n\n", i, addr->protocol,
			       	addr->host, addr->port, addr->abs_path,
			       	addr->query, addr->fragment);

		memset(url, 0, MAX);
		i++;
	}

	free(addr);

	return 0;

}

void *set_mem(size_t size){
	void *ret = NULL;

	ret = malloc(size);
	if(ret == NULL){
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	return ret;
}

void init_protocol_mem(struct web_addr *ptr, char *url, size_t size){
	ptr->protocol = set_mem(size+1);
	strncpy(ptr->protocol, url, size);
	ptr->protocol[size+1] = '\0';
}

void init_host_mem(struct web_addr *ptr, char *url, size_t size){
	ptr->host = set_mem(size+1);
	strncpy(ptr->host, url, size);
	ptr->host[size+1] = '\0';
}

void init_port_mem(struct web_addr *ptr, char *url, size_t size){
	ptr->port = set_mem(size+1);
	strncpy(ptr->port, url, size);
	ptr->port[size+1] = '\0';
}

void init_abs_path_mem(struct web_addr *ptr, char *url, size_t size){
	ptr->abs_path = set_mem(size+1);
	strncpy(ptr->abs_path, url, size);
	ptr->abs_path[size+1] = '\0';
}

void init_query_mem(struct web_addr *ptr, char *url, size_t size){
	ptr->query = set_mem(size+1);
	strncpy(ptr->query, url, size);
	ptr->query[size+1] = '\0';
}

void init_frag_mem(struct web_addr *ptr, char *url, size_t size){
	ptr->fragment = set_mem(size+1);
	strncpy(ptr->fragment, url, size);
	ptr->fragment[size+1] = '\0';
}
	
int parse_url(struct web_addr **addr, char *url){
	struct web_addr *ptr = NULL;
	int i = 0;
	char flag = NONE;

	if(url == NULL)
		return 0;

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
				init_host_mem(ptr, url, i);
				break;
			}
			if(flag == PORT){
				init_port_mem(ptr, url, i);
				break;
			}
			if(flag == ABS_PATH){
				init_abs_path_mem(ptr, url, i);
				break;
			}
			if(flag == QUERY){
				init_query_mem(ptr, url, i);
				break;
			}
			if(flag == FRAGMENT){
				init_frag_mem(ptr, url, i);
				break;
			}
		}
		if(url[i] == ':'){
			// is protocol
			if(url[i+1] == '/'){
				init_protocol_mem(ptr, url, i);
				flag = PROTOCOL;
				url += i+3;
				i = 0;
			}
			// is port
			else{
				init_host_mem(ptr, url, i);
				url += i+1;
				flag = PORT;
				i = 0;
			}
		}
		if(url[i] == '/'){
			if(flag == PORT){
				init_port_mem(ptr, url, i);
				url += i;
				i = 0;
			}
			if((flag == NONE) || (flag == PROTOCOL)){
				init_host_mem(ptr, url, i);
				url += i;
				i = 0;
			}
			flag = ABS_PATH;	
		}
		if(url[i] == '?'){
			init_abs_path_mem(ptr, url, i);
			url += i+1;
			flag = QUERY;
			i = 0;
		}
		if(url[i] == '#'){
			init_query_mem(ptr, url, i);
			url += i+1;
			flag = FRAGMENT;
			i = 0;
		}
		i++;
	}

	*addr = ptr;

	return 1;
}

void set_defaults(struct web_addr *ptr){
	if(ptr->protocol == NULL)
		init_protocol_mem(ptr, "http", strlen("http"));
	if(ptr->port == NULL)
		init_port_mem(ptr, "80", strlen("80"));
	if(ptr->abs_path == NULL)
		init_abs_path_mem(ptr, "/", strlen("/"));
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
