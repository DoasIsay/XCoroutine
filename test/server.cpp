#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include "scheduler.h"
#include "socket.h"

bool isExit = false;

int socketHandleCoroutine(void *arg){
    int fd = *(int*)arg;
    char buf[256];
    printf("%d:%d start socketHandleCoroutine fd: %d\n", gettid(), getcid(), fd);

    int ret = 0;
    while(!isExit){
        ret = net::readn(fd, buf, 19);
        if(ret <= 0)
            break;
        printf("%d:%d fd: %d, recv %s\n", gettid(), getcid(), fd, buf);
        net::writen(fd, buf, 19);
    }
    close(fd);
    return 0;
}

int acceptCoroutine(void *arg){
    int serverFd=*(int*)arg;
    int clientFd = 0;

    while(!isExit){
        if((clientFd = net::accept(serverFd)) > 2){
            printf("%d:%d accept fd %d\n", gettid(), getcid(), clientFd);
            createCoroutine(socketHandleCoroutine, &clientFd);
        }
    }
}

void quit(int signo)
{
	isExit = true;
}

int main(int argc, char** argv){
    
    signal(SIGTERM,quit);
    
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
    net::setNoBlock(serverFd);

    envInitialize();
    createCoroutine(acceptCoroutine, &serverFd);
    yield;
    envDestroy();

    close(serverFd);
    return 0;
}
