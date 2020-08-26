#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include "coroutine.h"
#include "log.h"

bool isExit = false;

int socketHandleCoroutine(void *arg){
    char buf[32];
    int fd = *(int*)arg;
    
    log(INFO, "start socketHandleCoroutine fd:%d", fd);
    
    while(!isExit){
        int ret = read(fd, buf, 19);
        if(ret <= 0)
            break;
        log(INFO, "fd:%d recv %s", fd, buf);
        
        ret = write(fd, buf, 19);
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
    int  serverFd;
    struct sockaddr_in  addr;

    if((serverFd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        log(ERROR, "create socket error: %s\n", strerror(errno));
        return 0;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(5566);
    if(bind(serverFd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        log(ERROR, "bind socket error: %s)\n", strerror(errno));
        return 0;
    }
    if(listen(serverFd, 10000) < 0){
        log(ERROR, "listen socket error: %s\n", strerror(errno));
        return 0;
    }


    while(!isExit){
        int *clientFd = new int;
        
        if((*clientFd = accept(serverFd, NULL, NULL)) > 2){
            log(INFO,"accept fd %d", *clientFd);
            createCoroutine(socketHandleCoroutine, (void*)clientFd);
        }else{
            log(ERROR, "accept error:%s", strerror(errno));
            break;
        }
        
    }
    
    close(serverFd);
}

void quit(int signo)
{
    isExit = true;
    stopCoroutines();
}

int main(int argc, char** argv){
    signal(SIGINT, quit);
    signal(SIGTERM, quit);
    signal(SIGPIPE, SIG_IGN);
    
    Coroutine *co = new Coroutine(acceptCoroutine, NULL);
    co->setPrio(1);
    yield;
    
    log(INFO, "exit sucess");
}

