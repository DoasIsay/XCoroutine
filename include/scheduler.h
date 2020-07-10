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

extern __thread Coroutine* last;

class Scheduler{
private:
    int idx;
    int running;
    volatile static int state;
    
    friend void addToSigQue(Coroutine *co);
    friend void addToTimerQue(Coroutine *co);
    
    Queue<Coroutine*> runQue[2][SCH_PRIO_SIZE];
    static Queue<Coroutine*, SpinLocker> sigQue;
    
    std::priority_queue<Coroutine*, std::vector<Coroutine*>, compare> timerQue;
    
    int epfd;
    int maxEventSize;
    epoll_event *events;

    pid_t groupid;
    
    Scheduler(int max);
    Scheduler() = delete;
    Scheduler(const Scheduler &) = delete;
    Scheduler &operator=(const Scheduler&) = delete;

    #ifdef STACK_SEPARATE
    char *stack;
    #endif

    static __thread Scheduler *instance;
    
public:
    
    static Scheduler* Instance(){
        if(!instance)
            instance = new Scheduler(1024);
        return instance;
    }

    void stop();
    
    void wakeup();
    
    int wait(int fd, int type, int timeout);
   
    void schedule();
    
    void runProcess();
    
    void timerProcess();
    
    void eventProcess();
    
    void signalProcess();
    
    pid_t getGrouid(){
        return groupid;
    }

    #ifdef STACK_SEPARATE
    char *getStack(){
        return stack;
    }
    #endif
    
    Coroutine* next();
    
    void addToRunQue(Coroutine *co){
        runQue[!running][co->getPrio()].push(co);
    }
    
    ~Scheduler();
};

void addToRunQue(Coroutine *co);
void addToSigQue(Coroutine *co);
void addToTimerQue(Coroutine *co);

void cexit(int status);
void startCoroutine();
void stopCoroutines();

static inline int wait(int fd, int type, int timeout){
    assert(current != NULL);
    if(save(current->getContext())){
    if(last != NULL){
        //now we can delete last coroutine safely on current coroutine stack
        delete last;
        last = NULL;
    }
    if(current->getIntr()){
        current->setIntr(false);
        return -1;
    }
    return 1;
    }
    
    STACK_OVERFLOW_CHECK(current->getStack(), current->getStackSize());

    #ifdef STACK_SEPARATE
    #ifdef STACK_CHECK
    long sp = (long)(Scheduler::Instance()->getStack() + SCH_STACK_SIZE - 8);
    #else
    long sp = (long)(Scheduler::Instance()->getStack() + SCH_STACK_SIZE);
    #endif
    //switch coroutine stack to scheduler stack
    #ifdef __i386__
    asm("movl %0,%%esp;"::"m"(sp));
    #elif __x86_64__
    asm("movq %0,%%rsp;"::"m"(sp));
    #endif
    #endif
    
    return Scheduler::Instance()->wait(fd, type, timeout);
}

static inline int waitOnRead(int fd){
    return wait(fd, EVENT::READABLE, 0);
}

static inline int waitOnWrite(int fd){
    return wait(fd, EVENT::WRITEABLE, 0);
}

static inline int waitOnTimer(int timeout){
    return wait(-1, -1, timeout);
}

static inline void schedule(){
    Scheduler::Instance()->schedule();
}

static inline void clear(){
    if(!current) return;
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
			current->start();\
		}\
		waitOnTimer(0);\
	}while(0)
	
#endif
