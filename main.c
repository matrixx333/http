#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFSIZE 4096
#define CRLF "\r\n"
#define EOL "\r\n\r\n"
#define NOT_CHUNKED 0x01
#define CHUNKED 0x02

struct host_data
{
	struct addrinfo *hints;
	struct addrinfo *res;
	struct sockaddr_in *sa;
	char *ip_addr;
	char *node;
	char *service;
	int error;
	int sock;
};

struct entity_data
{
	char *hdr;
	char *body;
};

typedef struct host_data HOST;
typedef struct entity_data ENTITY;

// wrapper functions
void *xmalloc(size_t size);
void *xrealloc(void *data, size_t size);
ssize_t xrecv(int socket, void *buffer, size_t length, int flags);
void xinet_ntop(HOST *data);

// socket functions
void setupConnect(HOST *data, char *node, char *service);
void initStruct(HOST *data, char *node, char *service);
void connectServer(HOST *data);

// http functions
void sendGET(HOST *data);
void recvEntity(HOST *data, ENTITY *entity);
char *getHeader(HOST *data);
int getChunkSize(HOST *data);
int getContentLen(ENTITY *entity);
char *removeCRLF(char *buf, int *size);
int determineEntBodyLen(ENTITY *entity);
void findURLs(ENTITY *entity);

// misc functions
void printInfo(HOST *data, ENTITY *entity);
void freePtr(HOST *data, ENTITY *entity);

int main(int argc, char **argv)
{
	HOST data;
	ENTITY entity;

	if (argc < 2)
	{
		printf("not enough cmd line args\n");
		exit(EXIT_FAILURE);
	}

	setupConnect(&data, argv[1], "http");
	sendGET(&data);
	recvEntity(&data, &entity);
	findURLs(&entity);
	// printInfo(&data, &entity);
	freePtr(&data, &entity);

	return 0;
}

// wrapper functions
void *xmalloc(size_t size)
{
	void *ptr = NULL;

	if ((ptr = malloc(size)) == NULL)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	return ptr;
}

void *xrealloc(void *data, size_t size)
{
	void *ptr = NULL;

	if ((ptr = realloc(data, size)) == NULL)
	{
		perror("realloc");
		exit(EXIT_FAILURE);
	}

	return ptr;
}

ssize_t xrecv(int socket, void *buffer, size_t length, int flags)
{
	ssize_t recvbytes = 0;
	size_t bytesleft = length;

	while (bytesleft > 0)
	{
		if ((recvbytes = recv(socket, buffer, bytesleft, flags)) <= 0)
		{
			if (recvbytes == 0)
				printf("Connection closed by remote host.\n");
			else
				perror("recv");

			close(socket);
			exit(EXIT_FAILURE);
		}

		bytesleft -= recvbytes;
		buffer += recvbytes;
	}

	return length - bytesleft;
}

void xinet_ntop(HOST *data)
{
	struct addrinfo *res = NULL;
	struct sockaddr_in *sa = NULL;

	res = data->res;
	sa = (struct sockaddr_in *)res->ai_addr;
	data->sa = sa;

	inet_ntop(res->ai_family, &(sa->sin_addr), data->ip_addr,
			  (socklen_t)res->ai_addrlen);
}

// socket functions
void setupConnect(HOST *data, char *node, char *service)
{
	initStruct(data, node, service);

	if ((data->error = getaddrinfo(node, service, data->hints,
								   &data->res)) != 0)
	{
		fprintf(stderr, "selectserver: %s\n", gai_strerror(data->error));
		exit(EXIT_FAILURE);
	}

	connectServer(data);
	xinet_ntop(data);
}

void initStruct(HOST *data, char *node, char *service)
{
	struct addrinfo *hints = NULL;

	data->hints = xmalloc(sizeof(struct addrinfo));
	data->sa = xmalloc(sizeof(struct sockaddr_in));
	data->ip_addr = xmalloc(INET_ADDRSTRLEN);
	data->node = node;
	data->service = service;
	data->sock = 0;
	data->error = 0;

	hints = data->hints;

	memset(hints, 0, sizeof(struct addrinfo));
	hints->ai_family = AF_UNSPEC;
	hints->ai_socktype = SOCK_STREAM;
	hints->ai_protocol = 0;
}

void connectServer(HOST *data)
{
	struct addrinfo *res = data->res;

	if ((data->sock = socket(res->ai_family, res->ai_socktype,
							 res->ai_protocol)) < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	if ((connect(data->sock, (const struct sockaddr *)res->ai_addr,
				 (socklen_t)res->ai_addrlen)) != 0)
	{
		perror("connect");
		exit(EXIT_FAILURE);
	}
}

// http functions
void sendGET(HOST *data)
{
	int bytessent = 0;
	char *message = NULL;
	int msgsize = 0;

	message = xmalloc(256);

	sprintf(message, "GET / HTTP/1.1\r\n"
					 "Host: %s\r\n"
					 "\r\n",
			data->node);

	msgsize = strlen(message);
	bytessent = send(data->sock, message, msgsize, 0);
}

void recvEntity(HOST *data, ENTITY *entity)
{
	char *ptr = NULL;
	char *buf = NULL;
	ssize_t recvbytes = 0;
	int chunkSize = 0;
	int bodySize = 0;
	int totalSize = 0;
	int encoding = 0;

	entity->hdr = getHeader(data);
	encoding = determineEntBodyLen(entity);

	if (encoding == CHUNKED)
	{
		while (1)
		{
			chunkSize = getChunkSize(data);
			if (chunkSize == 0)
				break;

			totalSize += chunkSize + 2;
			ptr = xrealloc(ptr, totalSize);
			buf = ptr + (totalSize - (chunkSize + 2));
			recvbytes = xrecv(data->sock, buf, chunkSize + 2, 0);
		}

		entity->body = removeCRLF(ptr, &totalSize);
		free(ptr);
		ptr = NULL;
		buf = NULL;
	}
	else
	{
		bodySize = getContentLen(entity);
		buf = xmalloc(bodySize);
		recvbytes = xrecv(data->sock, buf, bodySize, 0);
		entity->body = buf;
	}
}

char *getHeader(HOST *data)
{
	ssize_t recvbytes = 0;
	int i = 0;
	char buffer[BUFSIZE] = {0};
	char *eol = NULL;
	char *buf = NULL;

	while ((recvbytes = xrecv(data->sock, buffer + i, 1, 0)) > 0)
	{
		if (i > BUFSIZE)
			break;
		eol = strstr(buffer, EOL);
		if (eol != NULL)
		{
			*eol = '\0';
			buf = xmalloc(strlen(buffer) + 1);
			memcpy(buf, buffer, strlen(buffer) + 1);
			break;
		}
		i++;
	}

	return buf;
}

int getChunkSize(HOST *data)
{
	ssize_t recvbytes = 0;
	int chunkSize = 0;
	int i = 0;
	char size[8] = {0};
	char *eof = NULL;

	while ((recvbytes = xrecv(data->sock, size + i, 1, 0)) > 0)
	{
		if (i > 8)
			break;
		eof = strstr(size, CRLF);
		if (eof != NULL)
		{
			*eof = '\0';
			break;
		}
		i++;
	}
	sscanf(size, "%x", &chunkSize);

	return chunkSize;
}

int getContentLen(ENTITY *entity)
{
	char *ptr = NULL;
	char size[8] = {0};
	int i = 0;
	int bodySize = 0;

	ptr = strstr(entity->hdr, "Content-Length");

	ptr = strchr(ptr, ':');
	ptr += 2;
	while (1)
	{
		if (i > 8)
			break;
		if (size[i] == '\r')
		{
			size[i] = '\0';
			break;
		}
		size[i] = ptr[i];
		i++;
	}
	sscanf(size, "%d", &bodySize);

	return bodySize;
}

char *removeCRLF(char *buf, int *size)
{
	char *ptr = NULL;
	int i = 0;

	ptr = malloc(*size);

	while (i < *size)
	{
		if ((buf[i] == '\r') || (buf[i] == '\n'))
		{
			buf++;
			continue;
		}
		ptr[i] = buf[i];
		i++;
	}

	return ptr;
}

int determineEntBodyLen(ENTITY *entity)
{
	char *ptr = NULL;

	if ((ptr = strstr(entity->hdr, "Transfer-Encoding: chunked")))
		return CHUNKED;
	else
		return NOT_CHUNKED;
}

void findURLs(ENTITY *entity)
{
	int numurl = 0;
	char *ptr = NULL;
	char *buf = NULL;

	buf = entity->body;

	while (1)
	{
		if ((ptr = strstr(buf, "\"http")))
			numurl++;
		else
			break;

		buf = ptr;
		buf += 5;
	}
	printf("numurl: %d\n", numurl);
}

// misc functions
void printInfo(HOST *data, ENTITY *entity)
{
	printf("Host: %s\n"
		   "IP address: %s\n\n"
		   "%s\n\n"
		   "%s\n",
		   data->node, data->ip_addr, entity->hdr,
		   entity->body);

	close(data->sock);
}

void freePtr(HOST *data, ENTITY *entity)
{
	free(data->hints);
	free(data->sa);
	free(data->ip_addr);

	data->hints = NULL;
	data->res = NULL;
	data->sa = NULL;
	data->ip_addr = NULL;

	free(entity->hdr);
	free(entity->body);

	entity->hdr = NULL;
	entity->body = NULL;
}
