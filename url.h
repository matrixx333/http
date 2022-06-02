#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

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
void init_struct_elem(char **ptr, char *url, size_t size);
int checkurl(char *url);
int parse_url(struct web_addr **addr, char *url);
void set_defaults(struct web_addr *ptr);
void free_mem(struct web_addr *ptr);

