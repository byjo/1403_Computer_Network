#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define BUF_SIZE		1024
#define	MESSAGE_SIZE	128 
#define MAX_CLIENT		64

void addClientArr(int *clientArr, int *clientSize, int clientSock);
void deleteClientArr(int *clientArr, int *ClientSize, int clientSock);
void errorHandling(char *message);

int main(int argc, char *argv[])
{
	int serverSock, clientSock;
	struct sockaddr_in serverAddr, clientAddr;
	struct epoll_event *epollEvents;
	struct epoll_event epollEvent;
	int epfd, eventCount;
	socklen_t addrSize;
	char connectMessage[] = "Welcome! If you want to quit chatting service, enter 'q' or 'Q'\n";
	char rejectMessage[] = "Sorry, server is busy now. Please connect in a few times\n";
	char buf[BUF_SIZE];
	int messageLen;
	int clientArr[MAX_CLIENT];
	int clientSize, i;

	if (argc != 2)
		errorHandling("USAGE: ./chatServer <port>");

	serverSock = socket(PF_INET, SOCK_STREAM, 0);
	if (serverSock == -1)
		errorHandling("socket() Error: ");

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(atoi(argv[1]));

	if ( bind(serverSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1 )
		errorHandling("bind() Error: ");

	if ( listen(serverSock, 5) == -1 )
		errorHandling("listen() Error: ");

	system("clear");
	printf("server Activated\n");

	epfd = epoll_create(MAX_CLIENT);
	epollEvents = malloc(sizeof(struct epoll_event) * MAX_CLIENT);
	epollEvent.events = EPOLLIN;
	epollEvent.data.fd = serverSock;
	epoll_ctl(epfd, EPOLL_CTL_ADD, serverSock, &epollEvent);

	memset(clientArr, 0, MAX_CLIENT * sizeof(int));
	clientSize = 0;

	while(1){
		eventCount = epoll_wait(epfd, epollEvents, MAX_CLIENT, -1);
		
		if (eventCount == -1)
			printf("epoll_waite() Error\n");

		for (i = 0; i < eventCount; i++){

			if (epollEvents[i].data.fd == serverSock){
				addrSize = sizeof(clientAddr);
				clientSock = accept(serverSock, (struct sockaddr *)&clientAddr, &addrSize);

				if (clientSock == -1)
					printf("accpet() Error\n");

				if (clientSize < MAX_CLIENT - 1){
					//write(clientSock, connectMessage, MESSAGE_SIZE);

					epollEvent.events = EPOLLIN;
					epollEvent.data.fd = clientSock;
					epoll_ctl(epfd, EPOLL_CTL_ADD, clientSock, &epollEvent);

					addClientArr(clientArr, &clientSize, clientSock);
					
					printf("client[%d] Connect\n", clientSock);
					write(clientSock, connectMessage, MESSAGE_SIZE);
				}	

				else{
					write(clientSock, rejectMessage, MESSAGE_SIZE);
					close(clientSock);
				}
			}

			else{
				messageLen = read(epollEvents[i].data.fd, buf, BUF_SIZE);
				if (messageLen <= 0){
					epoll_ctl(epfd, EPOLL_CTL_DEL, epollEvents[i].data.fd, NULL);

					deleteClientArr(clientArr, &clientSize, epollEvents[i].data.fd);
			
					printf("client[%d] Disconnect\n", epollEvents[i].data.fd);
					close(epollEvents[i].data.fd);
				}

				else{
					for (i = 0; i < clientSize; i++)
						write(clientArr[i], buf, messageLen);
				}
			}
		}
	}

	close(serverSock);
	close(epfd);
	return 0;
}

void addClientArr(int *clientArr, int *clientSize, int clientSock)
{	
	clientArr[*clientSize] = clientSock;
	(*clientSize)++;
}

void deleteClientArr(int *clientArr, int *clientSize, int clientSock)
{	
	int i;

	for ( i = 0; i < *clientSize; i++ ){
		if ( clientArr[i] == clientArr[clientSock] )
			break;
	}

	clientArr[i] = clientArr[*clientSize];
	(*clientSize)--;
}

void errorHandling(char *message)
{
	perror(message);
	exit(1);
}
