#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>

#define BUF_SIZE	1024
#define NAME_SIZE	20	

void *readFunction(void *arg);
void *writeFunction(void *arg);
void ErrorHandling(char *message);

char name[NAME_SIZE];

int main(int argc, char*argv[])
{
	int sock;
	struct sockaddr_in serverAddr;
	pthread_t readTId, writeTId;
	void *threadReturn;

	if (argc != 4)
		ErrorHandling("USAGE : ./chatClient <IP> <port> <name>");

	sprintf (name, "[%s]", argv[3]);
	system("clear");

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if ( sock == -1 )
		ErrorHandling("socket() Error: ");

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
	serverAddr.sin_port = htons(atoi(argv[2]));

	if ( connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1 )
		ErrorHandling("connect() Error: ");
	
	if ( pthread_create(&readTId, NULL, readFunction, (void *)&sock) )
		ErrorHandling("pthread_create() Error: ");
	if ( pthread_create(&writeTId, NULL, writeFunction, (void *)&sock) )
		ErrorHandling("phtread_create() Error: ");

	if ( pthread_join(readTId, &threadReturn) )
		ErrorHandling("pthread_join() Error: ");
	if ( pthread_join(readTId, &threadReturn) )
		ErrorHandling("pthread_join() Error: ");
	
	return 0;
}

void *readFunction(void *arg)
{	
	int sock = *((int *)arg);
	int messageLen;
	char message[NAME_SIZE + BUF_SIZE];

	while(1){
		messageLen = read(sock, message, NAME_SIZE + BUF_SIZE - 1);

		if ( !messageLen )
			return (void *) -1;
		else if ( messageLen == -1 )
			printf("read() Error\n");

		message[messageLen] = 0;
		printf("%s", message);
	}

	return (void *) NULL;
}

void *writeFunction(void *arg)
{
	int sock = *((int *)arg);
	char buf[BUF_SIZE];
	char message[NAME_SIZE + BUF_SIZE];

	while(1){
		fgets(buf, BUF_SIZE, stdin);

		if ( !strcmp(buf, "q\n") || !strcmp(buf, "Q\n") ){
			close(sock);
			exit(0);
		}
		sprintf(message, "%s %s", name, buf);
		write(sock, message, strlen(message));
	}
	return (void *) NULL;
}

void ErrorHandling(char *message)
{
	perror(message);
	exit(1);
}

