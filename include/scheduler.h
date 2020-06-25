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
#include <queue>
#include "coroutine.h"
#include "queue.h"
#include "epoll.h"
#include "cormap.h"
#include "log.h"

class Scheduler{
private:
    friend void addToRunQue(Coroutine *co);
    Queue<Coroutine*> runQue;
    
    std::priority_queue<Coroutine*, std::vector<Coroutine*>, compare> timerQue;
    
    int epfd;
    int maxEventSize;
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
    
    int wait(int fd, int type, int timeout);
    
    int schedule();
    
    void runProcess();
    
    void timerProcess();
    
    void eventProcess();
    
    void signalProcess();
    
    ~Scheduler();
};

void addToRunQue(Coroutine *co);

void startCoroutine();

static inline int waitOnRead(int fd){
    Scheduler::Instance()->wait(fd, EVENT::READABLE, 0);
}

static inline int waitOnWrite(int fd){
    Scheduler::Instance()->wait(fd, EVENT::WRITEABLE, 0);
}

static inline int waitOnTimer(int timeout){
    Scheduler::Instance()->wait(-1, -1, timeout);
}

static inline void schedule(){
    Scheduler::Instance()->schedule();
}

static inline void clear(){
    if(!current)
        return;
    CorMap::Instance()->del(current->getcid());
    if(current->getEpfd() > 0){
        epollDelEvent(current->getEpfd(), current->getFd(), current->getType());
        current->setEpfd(-1);
    }
}

#define yield\
	do{\
		if(current == NULL){\
			current = new Coroutine();\
		}\
		waitOnTimer(0);\
	}while(0)
	
#endif
