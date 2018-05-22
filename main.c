#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define MAX_MESSAGE_LENGTH 512

int_fast16_t check;
uint8_t line[MAX_MESSAGE_LENGTH];

void error(uint8_t *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

int connect_server(void)
{
	struct addrinfo hints;
	struct addrinfo *result, *result_p;
	int socket_fd;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;

	check = getaddrinfo("irc.freenode.net", "6667", &hints, &result);
	if(check != 0)
	{
		error("getaddrinfo");
		gai_strerror(check);
	}

	for(result_p = result; result_p != NULL; result_p = result_p->ai_next)
	{
		socket_fd = socket(result_p->ai_family, result_p->ai_socktype,
				   result_p->ai_protocol);
		if(socket_fd == 0)
		{
			error("socket");
			break;
		}

		struct sockaddr_in *addr = ((struct sockaddr_in *)
					    result_p->ai_addr);
		char ip[INET6_ADDRSTRLEN];
		int port = ntohs(addr->sin_port);

		inet_ntop(result_p->ai_family, &addr->sin_addr, ip, INET6_ADDRSTRLEN);
		printf("ip: %s; port: %d; protocol: %d\n",
		       ip, port, result_p->ai_protocol);

		check = connect(socket_fd, result_p->ai_addr,
				result_p->ai_addrlen);
		if(check == 0)
		{
			break;

		}

		error("connect");
		close(socket_fd);		
	}
	freeaddrinfo(result);

	printf("OK\n");
	return socket_fd;
}

void readline(int socket_fd, uint8_t *line)
{
	char c;
	int_fast16_t i;

	printf("readline begin\n");
	
	for(i = 0; i <= MAX_MESSAGE_LENGTH; i++)
	{
		check = recv(socket_fd, c, 1, 0);
		if(check < 0)
		{
			printf("%d", i);
			error("recv");
		}

		if(c == '\r')
		{
			if((recv(socket_fd, c, 1, 0)) == '\n')
			{
				*line = '\0';
				printf("break\n");
				break;
			}
		}else
			*line++ = c;
	}

	printf("readline end\n");
}

int main(int argc, char *argv[])
{
	int socket_fd;
	uint8_t buffer[MAX_MESSAGE_LENGTH];
	int_fast16_t i;
	
	socket_fd = connect_server();
	for(i = 0; i < 2; i++)
	{
	check = recv(socket_fd, buffer, MAX_MESSAGE_LENGTH, 0);
	if(check < 0)
	  error("read");
	printf("%s\n", buffer);
	}
	
	strncpy(buffer, "NICK tastytestbot\r\n", MAX_MESSAGE_LENGTH);
	printf("%s #%d#\n", buffer, strlen(buffer));
	check = send(socket_fd, buffer, strlen(buffer), 0);
	if(check < 0)
		error("send NICK");
	
	strncpy(buffer, "USER tastytestbot 0 * :tastytestbot\r\n",
		MAX_MESSAGE_LENGTH);
	printf("%s #%d#\n", buffer, strlen(buffer));
	check = send(socket_fd, buffer, strlen(buffer), 0);
	if(check < 0)
		error("send USER");
	
	while(1)
	{
		check = recv(socket_fd, buffer, strlen(buffer), 0);
		if(check < 0)
			error("read");
		printf("%s\n", buffer);
	}
	int_fast8_t b = 0;
	/* while(b < 2) */
	/* { */
	/* 	readline(socket_fd, line); */
	/* 	printf("%s\n", line); */
	/* 	b = 2; */
	/* } */
	
	printf("OK");
	return 0;
}
