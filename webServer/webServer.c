#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#define BUF_SIZE			4096
#define PORT				32000
#define LISTEN_QUEUE_SIZE	32

void errorHandling(char *message);

int main(int argc, char *argv[])
{
	int serverSock, clientSock;
	struct sockaddr_in serverAddr, clientAddr;
	socklen_t addrSize;
	char header[] = "HTTP/1.1 200 OK\r\nDate: Wed, 12 Mar 2014 00:14:10 GMT\r\n\r\n";
	char errorHeader[] = "HTTP/1.1 404 Not Found\r\n\r\n";
	char buf[BUF_SIZE];
	char *file;
	int messageLen, fd, n;

	if (argc != 1)
		errorHandling("USAGE: ./webServer");

	serverSock = socket(PF_INET, SOCK_STREAM, 0);
	if (serverSock == -1)
		errorHandling("socket() Error: ");

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(PORT);

	if ( bind(serverSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1 )
		errorHandling("bind() Error: ");

	if ( listen(serverSock, LISTEN_QUEUE_SIZE) == -1 )
		errorHandling("listen() Error: ");

	system("clear");
	printf("server Activated\n");

	while(1){
		addrSize = sizeof(clientAddr);
		clientSock = accept(serverSock, (struct sockaddr *)&clientAddr, &addrSize);

		if (clientSock == -1)
			printf("accpet() Error\n");

		messageLen = recv(clientSock, buf, BUF_SIZE, 0);
		buf[messageLen] = 0;
		printf("=====\n%s=====\n", buf);

		strtok(buf, "/");
		file = strtok(NULL, " ");
		
		printf("%s\n", file);
		fd = open(file, O_RDONLY);

		if (fd == -1){
			send(clientSock, errorHeader, strlen(errorHeader), 0);
			fd = open("error404.html", O_RDONLY);
		}
		else
			send(clientSock, header, strlen(header), 0);


		while ( (n = read(fd, buf, BUF_SIZE)) > 0 )
			send(clientSock, buf, n, 0);

		close(fd);
		close(clientSock);
	}

	close(serverSock);
	return 0;
}

void errorHandling(char *message)
{
	perror(message);
	exit(1);
}
