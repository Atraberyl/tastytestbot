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

int connect_server(int *socket_fd)
{
	struct addrinfo hints;
	struct addrinfo *result, *result_p;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;

	check = getaddrinfo("irc.freenode.net", "6667", &hints, &result);
	if(check != 0)
	{
		perror("getaddrinfo");
		gai_strerror(check);
		return -1;
	}

	for(result_p = result; result_p != NULL; result_p = result_p->ai_next)
	{
		*socket_fd = socket(result_p->ai_family, result_p->ai_socktype,
				   result_p->ai_protocol);
		if(*socket_fd == 0)
		{
			perror("socket");
			break;
		}

		struct sockaddr_in *addr = ((struct sockaddr_in *)
					    result_p->ai_addr);
		char ip[INET6_ADDRSTRLEN];
		int port = ntohs(addr->sin_port);

		inet_ntop(result_p->ai_family, &addr->sin_addr, ip,
			  INET6_ADDRSTRLEN);
		printf("ip: %s; port: %d; protocol: %d\n",
		       ip, port, result_p->ai_protocol);

		check = connect(*socket_fd, result_p->ai_addr,
				result_p->ai_addrlen);
		if(check == 0)
			break;

		close(*socket_fd);		
	}
	if(result_p == NULL)
	{
		perror("connect");
		return -1;
	}
	freeaddrinfo(result);

	return 0;
}

void readline(int socket_fd, uint8_t *line)
{
	char c;
	int_fast16_t i;

	for(i = 0; i <= MAX_MESSAGE_LENGTH; i++)
	{
		check = recv(socket_fd, &c, 1, 0);
		if(check < 0)
		{
			error("recv");
		}

		if(c != '\r')
		{
			line[i] = c;
		}else
		{	
			check = recv(socket_fd, &c, 1, MSG_PEEK);
			if(check < 0)
				error("recv peek");
			
			if(c == '\n')
			{
				line[i] = '\0';
				break;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	int socket_fd;
	uint8_t buffer[MAX_MESSAGE_LENGTH];
	
	check = connect_server(&socket_fd);
	if(check < 0)
	{
		printf("Error connecting to server");
		exit(EXIT_FAILURE);
	}

	//Send Identification messages
	strncpy(buffer, "NICK tastytestbot\r\n", MAX_MESSAGE_LENGTH);
	check = send(socket_fd, buffer, strlen(buffer), 0);
	if(check < 0)
		error("send NICK");
	strncpy(buffer, "USER tastytestbot 0 * :tastytestbot\r\n",
		MAX_MESSAGE_LENGTH);
	check = send(socket_fd, buffer, strlen(buffer), 0);
	if(check < 0)
		error("send USER");

	while(1)
	{
		readline(socket_fd, line);
		printf("%s", line);
	}
	
	return 0;
}
