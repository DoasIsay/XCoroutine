/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#include <sys/syscall.h>
#include "scheduler.h"
#include "csignal.h"
#include <time.h>

#define INTHZ 10

__thread Coroutine *last = NULL;

__thread Scheduler* Scheduler::instance = NULL;

Queue<Coroutine*, SpinLocker> Scheduler::sigQue;

volatile int Scheduler::state = DEAD;

Scheduler::Scheduler(int max =1024){
    idx = 0;
    state = NEW;
    running = 0;
    
    maxEventSize = max;
    epfd = epollCreate(max);
    events = (epoll_event*)malloc(max*sizeof(epoll_event));
    
    groupid = syscall(__NR_gettid);
    
    #ifdef STACK_SEPARATE
    stack = allocMem(SCH_STACK_SIZE);
    #endif
}

#define setEvent(epfd, fd, type)\
        do{\
            current->setFd(fd);\
            current->setType(type);\
            current->setEpfd(epfd);\
        }while(0)

int Scheduler::wait(int fd, int type, int timeout){
    if(fd > 0){
        current->setState(WAITING);
        if(current->getEpfd() < 0){
            CorMap::Instance()->del(current->getcid());
            setEvent(epfd, fd, type);
            epollAddEvent(epfd, fd, type);
            CorMap::Instance()->set(fd, current);
        }else if(current->getType() != type){
            current->setType(type);
            epollModEvent(epfd, fd, type);
        }
    }else if(timeout <= 0){
        current->setState(RUNNABLE);
        addToRunQue(current);
    }
    
    if(timeout > 0){
        current->setTimeout(timeout);
        current->setState(WAITING);
        timerQue.push(current);
    }
    
    STACK_OVERFLOW_CHECK;
    
    schedule();
}

inline void Scheduler::wakeup(Coroutine *co){
    current = co;
    assert(co != NULL);
    assert(co->getState() != DEAD);
    
    if(getcid() == 0)
        state = DEAD;
    else if(state != STOPPED &&
        state != DEAD){
        state = RUNNABLE;
    }
    
    STACK_OVERFLOW_CHECK;
    co->setState(RUNNING);
    restore(co->getContext(), (long)doSignal);
}

Coroutine* Scheduler::next(){
    Coroutine *co = NULL;
    
    for(; idx < SCH_PRIO_SIZE; idx++){
        if(runQue[running][idx].empty())
            continue;
        co = runQue[running][idx].pop();
        break;
    }
    idx = 0;
    running = !running;

    return_check(co);
}

void Scheduler::runProcess(){
    Coroutine *co= next();
    if(co == NULL) return_check();

    if(co->getcid() != 0 ||
        CorMap::Instance()->empty()){
        wakeup(co);
    }else{
        addToRunQue(co);
    }
    
    return_check();
}

inline void Scheduler::timerProcess(){
    Coroutine *co = NULL;
    if(state != RUNNING || timerQue.empty())
        return_check();
    
    co = timerQue.top();
    if(co->getTimeout() > time(NULL))
        return_check();
    
    timerQue.pop();
    co->setState(RUNNABLE);
    co->setTimeout(0);
    addToRunQue(co);
    
    return_check();
}

void Scheduler::eventProcess(){  
    if(state != RUNNING) return_check();
    
    int firedEventSize = epollWait(epfd,
                                   events,
                                   maxEventSize,
                                   INTHZ);
    int nextEventIdx = 0;
    for(; nextEventIdx < firedEventSize; nextEventIdx++){
        Coroutine *co = NULL;
        
        co = (Coroutine*)events[nextEventIdx].data.ptr;
        assert(co->getState() != DEAD);
        co->setState(RUNNABLE);
        addToRunQue(co);
    }
    
    return_check();
}

void Scheduler::signalProcess(){
    Coroutine *co = sigQue.pop();
    if(co == NULL) return_check();
    
    if(true || co->getGroupid() == groupid){
        if(co->getState() == WAITING) co->setIntr(true);
        
        co->setState(RUNNABLE);
        assert(co->getGroupid() != 0);
        addToRunQue(co);
    }else{
        sigQue.push(co);
    }
    
    return_check();
}

void Scheduler::schedule(){
    if(state != STOPPED && state != DEAD)
        state = RUNNING;
    
    while(true){
        runProcess();
        timerProcess();
        eventProcess();
        signalProcess();
    }
}

void Scheduler::stop(){
    state = STOPPED;
    
    while(true){
        Coroutine *co = CorMap::Instance()->next();
        if(co == NULL) return_check();
        co->stop();
    }
}

Scheduler::~Scheduler(){
    close(epfd);
    free(events);
    #ifdef STACK_SEPARATE
    freeMem(stack, SCH_STACK_SIZE);
    #endif
}

void addToRunQue(Coroutine *co){
    Scheduler::Instance()->addToRunQue(co);
    return_check();
}

void addToSigQue(Coroutine *co){
    Scheduler::Instance()->sigQue.push(co);
    return_check();
}

void addToTimerQue(Coroutine *co){
    Scheduler::Instance()->timerQue.push(co);
    return_check();
}

int waitOnTimer(int timeout){
    return_check(wait(-1, -1, timeout));
}

void wakeup(Coroutine *co){
    Scheduler::Instance()->wakeup(co);
    return_check();
}

void stopCoroutines(){
    Scheduler::Instance()->stop();
    return_check();
}

void cexit(int status){
    STACK_OVERFLOW_CHECK;
    current->setState(DEAD);
    clear();
    switch(status){
        case -1:
        case  0:
        default:break;
    }
    
    #ifdef STACK_SEPARATE
    #ifdef STACK_CHECK
    long sp = (long)(Scheduler::Instance()->getStack() + SCH_STACK_SIZE);
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
    if(last != NULL){
        //now we can delete last coroutine safely on current coroutine stack
        delete last;
    }
    last = current;
    schedule();
}

void startCoroutine(){
    cexit(current->routine(current->arg));
}
