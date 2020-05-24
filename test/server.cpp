#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "coroutine.h"
#include "scheduler.h"
#include "socket.h"

int socketHandleCoroutine(void *arg){
	int fd = *(int*)arg;
	char buf[256];
	printf("%d start socketHandleCoroutine fd: %d\n", syscall(__NR_gettid), fd);

	int ret = 0;
	while(1){
		ret = NET::readn(fd, buf, 19);
		if(ret <= 0)
			break;
		printf("fd: %d, recv %s\n", fd, buf);
		NET::writen(fd, buf, 19);
	}
	close(fd);
	return ret;
}

int acceptCoroutine(void *arg){
	int serverFd=*(int*)arg;
    int clientFd = 0;

	while(1){
		if((clientFd = NET::accept(serverFd)) > 2){
			printf("%d accept fd %d\n", syscall(__NR_gettid), clientFd);
			createCoroutine(socketHandleCoroutine, &clientFd);
		}
	}
}

int main(int argc, char** argv){
	int  serverFd, connfd;
	struct sockaddr_in  servaddr;

	if((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		printf("create socket error: %s\n", strerror(errno));
		return 0;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(5566);
	if(bind(serverFd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
		printf("bind socket error: %s)\n", strerror(errno));
		return 0;
	}
	if(listen(serverFd, 10) == -1){
		printf("listen socket error: %s\n", strerror(errno));
		return 0;
	}

	NET::setNoBlock(serverFd);

	envInitialize();
	createCoroutine(acceptCoroutine, &serverFd);
	schedule();
	envDestroy();

	close(serverFd);
	return 0;
}
