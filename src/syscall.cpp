/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#include <time.h>
#include "syscall.h"
#include "scheduler.h"

typedef ssize_t (*SysRead)(int fd, void *buf, size_t count);
typedef ssize_t (*SysWrite)(int fd, const void *buf, size_t count);
typedef int (*SysAccept)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
typedef int (*SysClose)(int fd);
typedef unsigned int (*SysSleep)(unsigned int seconds);


SysRead sysRead	= (SysRead)dlsym(RTLD_NEXT, "read");
SysWrite sysWrite = (SysWrite)dlsym(RTLD_NEXT, "write");
SysAccept sysAccept = (SysAccept)dlsym(RTLD_NEXT, "accept");
SysClose sysClose = (SysClose)dlsym(RTLD_NEXT, "close");
SysSleep sysSleep = (SysSleep)dlsym(RTLD_NEXT, "sleep");


ssize_t read(int fd, void *buf, size_t count){
    int ret = sysRead(fd, buf, count);
    if(!current)
        return ret;
    
    setErno(errno);
    if(ret < 0){
        if(errno == EAGAIN){
            waitOnRead(fd);
        }
    }
    return ret;
}

ssize_t write(int fd, const void *buf, size_t count){
    int ret = sysWrite(fd, buf, count);
    if(!current)
        return ret;
    
    setErno(errno);
    if(ret < 0){
        if(errno == EAGAIN){
            waitOnWrite(fd);
        }
    }
    return ret;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    int ret = sysAccept(sockfd, addr, addrlen);
    if(!current)
        return ret;
    
    setErno(errno);
    if(ret < 0){
        if(errno == EAGAIN){
            waitOnRead(sockfd);
        }
    }
    return ret;
}

int close(int fd){
    if(current != NULL)
        clear();
    
    return sysClose(fd);
}

unsigned int sleep(unsigned int seconds){
    if(!current)
        return sysSleep(seconds);
    else{
        waitOnTimer(seconds + time(NULL));
    }
}

