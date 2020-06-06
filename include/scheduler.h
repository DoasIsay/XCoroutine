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
#include "log.h"

class Scheduler{
private:
    int epfd;
    int nextEventIdx;
    int firedEventSize;
    int maxEventSize;
    epoll_event *events;
    
    Queue<Coroutine*> runQue;
    
    static __thread Scheduler *instance;
    
    Scheduler(int max);
    Scheduler() = delete;
    Scheduler(const Scheduler &) = delete;
    Scheduler &operator=(const Scheduler&) = delete;
    
public:
    static Scheduler* Instance(){
        if(!instance)
            instance = new Scheduler(1024);
        return instance;
    }

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

#define yield\
	do{\
		if(current == NULL){\
			current = new Coroutine(0);\
		}\
		Scheduler::Instance()->wait(-1, -1);\
	}while(0)
	
#endif
