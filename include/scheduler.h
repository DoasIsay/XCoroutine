/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __SCHEDULER__
#define __SCHEDULER__

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include "coroutine.h"
#include "queue.h"
#include "epoll.h"
#include "cormap.h"
#include "log.h"

class Scheduler{
private:
    Queue<Coroutine*> *runQue;
    
    int epfd;
    int nextEventIdx;
    int maxEventSize;
    int firedEventSize;
    epoll_event *events;

    Scheduler(int max);
    Scheduler() = delete;
    Scheduler(const Scheduler &) = delete;
    Scheduler &operator=(const Scheduler&) = delete;

    static __thread Scheduler *instance;
    
public:
    static Scheduler* Instance(){
        if(!instance)
            instance = new Scheduler(1024);
        return instance;
    }
    
    void wakeup();
    
    int wait(int fd, int type);
    
    int schedule();
    
    Coroutine* next();
    
    void signalProcess();
    
    void timerInterrupt();
    
    void addToRunQue(Coroutine* co){
        runQue->push(co);
    }
    
    Coroutine* delFromRunQue(){
        return runQue->pop();
    }
    
    ~Scheduler();
};

void addToRunQue(Coroutine *co);

void startCoroutine();

static inline int waitOnRead(int fd){
    Scheduler::Instance()->wait(fd, EVENT::READABLE);
}

static inline int waitOnWrite(int fd){
    Scheduler::Instance()->wait(fd, EVENT::WRITEABLE);
}

static inline void schedule(){
    Scheduler::Instance()->schedule();
}

#define setEvent(epfd, fd, type)\
        do{\
            current->setFd(fd);\
            current->setType(type);\
            current->setEpfd(epfd);\
        }while(0)

static inline void clear(){
    if(current != NULL && current->getEpfd() > 0){
        CorMap::Instance()->del(current->getcid());
        epollDelEvent(current->getEpfd(), current->getFd(), current->getType());
        setEvent(-1, -1, -1);
    }
}

#define yield\
	do{\
		if(current == NULL){\
			current = new Coroutine(0);\
		}\
		Scheduler::Instance()->wait(-1, -1);\
	}while(0)
	
#endif
