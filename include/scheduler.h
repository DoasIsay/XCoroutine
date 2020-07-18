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
#include "context.h"
#include "memory.h"
#include "cormap.h"
#include "queue.h"
#include "epoll.h"
#include "log.h"


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
    
    void wakeup(Coroutine *co);
    
    int  wait(int fd, int type, int timeout);
   
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

void stopCoroutines();

extern __thread Coroutine *last;

static inline int wait(int fd, int type, int timeout){
    int ret = 0;
    assert(current != NULL);
    if(save(current->getContext())){
        if(current->getIntr()){
            current->setIntr(false);
            return_check(-1);
        }
        return_check(1);
    }

    #ifdef STACK_SEPARATE
    long sp = (long)(Scheduler::Instance()->getStack() + SCH_STACK_SIZE);
    //switch coroutine stack to scheduler stack
    #ifdef __i386__
    asm("movl %0,%%esp;"::"m"(sp));
    #elif __x86_64__
    asm("movq %0,%%rsp;"::"m"(sp));
    #endif
    #endif
    
    Scheduler::Instance()->wait(fd, type, timeout);
}

static inline int waitOnRead(int fd){
    return_check(wait(fd, EVENT::READABLE, 0));
}

static inline int waitOnWrite(int fd){
    return_check(wait(fd, EVENT::WRITEABLE, 0));
}

int  waitOnTimer(int timeout);

void wakeup(Coroutine *co);

static inline void schedule(){
    Scheduler::Instance()->schedule();
}


static inline void clear(){
    if(!current) return_check();
    CorMap::Instance()->del(current->getcid());
    if(current->getEpfd() > 0){
        epollDelEvent(current->getEpfd(), current->getFd(), current->getType());
        current->setEpfd(-1);
    }
    return_check();
}

void cexit(int status);
void startCoroutine();

#endif
