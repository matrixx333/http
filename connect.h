struct _structptr{
	struct addrinfo *hints;
	struct addrinfo *res;
	struct sockaddr_in *sa;
	char *ip_addr;
	char *node;
	char *service;
	int sock;
	int error; 
}; 

void setupConnect(struct _structptr *ptr, char *node, char *service);
void initStruct(struct _structptr *ptr, char *node, char *service);
void *heap(size_t size);
void connectServer(struct _structptr *ptr);
void convertNtop(struct _structptr *ptr);
void freePtr(struct _structptr *ptr);
