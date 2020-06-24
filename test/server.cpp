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
    char buf[256];
    int fd = *(int*)arg;
    
    log(INFO, "start socketHandleCoroutine fd:%d", fd);
    
    while(!isExit){
        int ret = net::readn(fd, buf, 19);
        if(ret <= 0)
            break;
        log(INFO, "fd:%d recv %s", fd, buf);
        
        ret = net::writen(fd, buf, 19);
        if(ret <= 0){
            log(ERROR, "fd:%d write error:%s\n", fd, strerror(errno));
            break;
        }
    }
    close(fd);
    delete (int*)arg;
    
    return 0;
}

int acceptCoroutine(void *arg){
    int serverFd=*(int*)arg;

    while(!isExit){
        int *clientFd = new int;
        
        if((*clientFd = net::accept(serverFd)) > 2){
            log(INFO,"accept fd %d", *clientFd);
            createCoroutine(socketHandleCoroutine, (void*)clientFd);
        }else{
            log(ERROR, "accept error:%s", strerror(errno));
            return -1;
        }
    }
}

void quit(int signo)
{
	isExit = true;
}

int main(int argc, char** argv){
    
    signal(SIGTERM,quit);
    
    int  serverFd;
    struct sockaddr_in  addr;

    if((serverFd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("create socket error: %s\n", strerror(errno));
        return 0;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(5566);
    if(bind(serverFd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        printf("bind socket error: %s)\n", strerror(errno));
        return 0;
    }
    if(listen(serverFd, 10) < 0){
        printf("listen socket error: %s\n", strerror(errno));
        return 0;
    }
    net::setNoBlock(serverFd);

    createCoroutine(acceptCoroutine, &serverFd);
    yield;
    
    close(serverFd);
    
    log(INFO, "exit sucessfully");
}

