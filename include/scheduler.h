/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __SCHEDULER__
#define __SCHEDULER__
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <assert.h>
#include "context.h"
#include "coroutine.h"
#include "queue.h"
#include "epoll.h"


class Scheduler{
private:
    int epollFd;
    int nextEventIdx;
    int firedEventSize;
    int maxEventSize;
    epoll_event *events;
    Queue<Coroutine*> runQue;
    
public:
    
    Scheduler(int max);

    Coroutine* next();
    
    int wait(int fd, int type);
    
    void addToRunQue(Coroutine* co){
        runQue.push(co);
    }
    Coroutine* delFromRunQue(){
        return runQue.pop();
    }
    
    void timerInterrupt();
    
    int schedule();

    void signalProcess();
    
    void wakeup();

    ~Scheduler();

};

extern __thread Scheduler *scheduler;

void addToRunQue(Coroutine *co);

void startCoroutine();

static inline int waitOnRead(int fd){
    scheduler->wait(fd, EVENT::READABLE);
}

static inline int waitOnWrite(int fd){
    scheduler->wait(fd, EVENT::WRITEABLE);
}

static inline void schedule(){
    scheduler->schedule();
}

#define yield\
	do{\
		if(current == NULL){\
			current = new Coroutine(0);\
		}\
		scheduler->wait(-1, -1);\
	}while(0)
#endif
