#include <stdio.h>
#include <string.h>

int main(int argc, char **argv){
	char buf[32] = {0};
	char *ptr = NULL;
	
	ptr = buf;
	
	strcpy(ptr, "this is a test string\n");
	
	printf("%s\n", ptr);
	
	ptr++;
	
	printf("%s\n", ptr);
	
	return 0;
}