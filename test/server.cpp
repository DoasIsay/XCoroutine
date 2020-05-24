#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h> 
#include <time.h>
#include "coroutine.h"
#include "scheduler.h"
#include "socket.h"

int socketHandleCoroutine(void *arg){
	int fd = *(int*)arg;
	char buf[256];
	printf("%d start socketHandleCoroutine fd: %d\n", getpid(), fd);
	while(1){
		int ret = readn(fd, buf, 19);
		if(ret <=0 )
			break;
		printf("fd: %d, recv %s\n", fd, buf);
		writen(fd, buf, 19);
	}
	close(fd);
}

int acceptCoroutine(void *arg){
	int serverFd=*(int*)arg;
	int clientFd = 0;
	JmpBuf jmpBuf;
	setjmp(&jmpBuf);
	while(1){
		printf("%d accept fd %d\n", getpid(), serverFd);
		if( (clientFd = accept(serverFd, (struct sockaddr*)NULL, NULL)) <= 0){
			if(errno==EAGAIN){
				printf("%d schedule to next\n", getpid());
				schedule(Scheduler::READ);
				continue;
			}else{
				printf("accept error: %s\n", strerror(errno));
				return -1;
			}	
		}
		else{
			current->setJmp(jmpBuf);
			schedule(Scheduler::READ, 0);
			setNoBlock(clientFd);
			createCoroutine(socketHandleCoroutine, &clientFd);
		}
	}
}

int main(int argc, char** argv){
	int  serverFd, connfd;
	struct sockaddr_in  servaddr;
  
	if( (serverFd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
		printf("create socket error: %s\n", strerror(errno));
		return 0;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(5566);
	if( bind(serverFd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
		printf("bind socket error: %s)\n", strerror(errno));
		return 0;
	}
	if(listen(serverFd, 10) == -1){
		printf("listen socket error: %s\n", strerror(errno));
		return 0;
	}
	setNoBlock(serverFd);
	
	envInitialize();
	createCoroutine(acceptCoroutine, &serverFd);
	envDestroy();

	close(serverFd);
	return 0;
}
