#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "connect.h"

int main(int argc, char **argv){
	struct _structptr ptr; 
	
	if(argc < 2) {
		printf("not enough cmd line args\n");
		exit(EXIT_FAILURE);
	}
	
	setupConnect(&ptr, argv[1], "http");
	
	printf("addr: %s\n", ptr.ip_addr);
	
	freePtr(&ptr);	

	return 0;
}
