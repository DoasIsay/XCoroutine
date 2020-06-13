/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __EPOLL__
#define __EPOLL__
#include <sys/epoll.h>
#include <errno.h>
#include <string.h>
#include "log.h"

namespace EVENT{
    enum {READABLE = EPOLLIN, WRITEABLE = EPOLLOUT};
};

class Coroutine;
extern __thread Coroutine *current;

static inline int epollCreate(int max){
    return epoll_create(max);
}

static inline int eventCtl(int epollFd, int fd, int type, int mode){
    epoll_event event;
    event.events = type|EPOLLET;
    if(type == EVENT::WRITEABLE)    event.events |= EPOLLONESHOT;
    event.data.ptr = (void*)current;
    int ret = epoll_ctl(epollFd, mode, fd, &event);
    if(ret < 0){
        log(ERROR,"epoll epfd:%d fd:%d type:%d mode:%d error: %s", epollFd, fd, type, mode, strerror(errno));
    }
    return ret;
}

static inline int epollAddEvent(int epollFd, int fd, int type){
    return eventCtl(epollFd, fd, type, EPOLL_CTL_ADD);
}

static inline int epollDelEvent(int epollFd, int fd, int type){
    return eventCtl(epollFd, fd, type, EPOLL_CTL_DEL);
}

static inline int epollModEvent(int epollFd, int fd, int type){
    return eventCtl(epollFd, fd, type, EPOLL_CTL_MOD);
}

static inline int epollWait(int epollFd, epoll_event *events, int maxEventSize, int inthz){
    return epoll_wait(epollFd, events, maxEventSize, inthz);
}

#endif
