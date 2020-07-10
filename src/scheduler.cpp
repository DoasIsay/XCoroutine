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

__thread Coroutine* last = NULL;
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
    stack = allocStack(SCH_STACK_SIZE);
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
    
    #ifdef STACK_SEPARATE
    STACK_OVERFLOW_CHECK(stack, SCH_STACK_SIZE);
    #else
    STACK_OVERFLOW_CHECK(current->getStack(), current->getStackSize());
    #endif
    
    schedule();
}

void Scheduler::signalProcess(){
    Coroutine *co = sigQue.pop();
    if(co == NULL) goto out;
    
    if(co->getGroupid() == groupid){
        if(co->getState() == WAITING) co->setIntr(true);
        co->setState(RUNNABLE);
        assert(co->getGroupid() != 0);
        addToRunQue(co);
    }else{
        sigQue.push(co);
    }
    
out:
    #ifdef STACK_SEPARATE
    STACK_OVERFLOW_CHECK(stack, SCH_STACK_SIZE);
    #else
    STACK_OVERFLOW_CHECK(current->getStack(), current->getStackSize());
    #endif
}

inline void Scheduler::wakeup(){
    assert(current != NULL);
    assert(current->getState() != DEAD);
    
    if(getcid() == 0) state = DEAD;
    else if(state != STOPPED &&
        state != DEAD){
        state = RUNNABLE;
    }
    
    current->setState(RUNNING);
    
    /* first time run on main coroutine stack, check will failed
    #ifdef STACK_SEPARATE
    STACK_OVERFLOW_CHECK(stack, SCH_STACK_SIZE);
    #else
    STACK_OVERFLOW_CHECK(current->getStack(), current->getStackSize());
    #endif
    */
    
    restore(current->getContext(), (long)doSignal);
}

Coroutine* Scheduler::next(){
    Coroutine *co = NULL;
    if(SCH_PRIO_SIZE == 1){
        if(!runQue[running][0].empty()) co = runQue[running][0].pop();
        if(co == NULL) running = !running;
        goto out;
    }
        
    for(; idx < SCH_PRIO_SIZE; idx++){
        if(runQue[running][idx].empty()) continue;
        co = runQue[running][idx].pop();
        goto out;
    }
    idx = 0;
    running = !running;
    
out:
    #ifdef STACK_SEPARATE
    STACK_OVERFLOW_CHECK(stack, SCH_STACK_SIZE);
    #else
    STACK_OVERFLOW_CHECK(current->getStack(), current->getStackSize());
    #endif
    
    return co;
}
void Scheduler::runProcess(){
    Coroutine *co= next();
    if(co == NULL) goto out;

    if(co->getcid() != 0 ||
        CorMap::Instance()->empty()){
        current = co;
        wakeup();
    }else{
        addToRunQue(co);
    }

out:
    #ifdef STACK_SEPARATE
    STACK_OVERFLOW_CHECK(stack, SCH_STACK_SIZE);
    #else
    STACK_OVERFLOW_CHECK(current->getStack(), current->getStackSize());
    #endif
}

void Scheduler::eventProcess(){
    int firedEventSize = 0;
    int nextEventIdx = 0;
    
    if(state != RUNNING) goto out;
    
    firedEventSize = epollWait(epfd, events, maxEventSize, INTHZ);
    for(; nextEventIdx < firedEventSize; nextEventIdx++){
        Coroutine *co = (Coroutine*)events[nextEventIdx].data.ptr;
        assert(co->getState() != DEAD);
        co->setState(RUNNABLE);
        addToRunQue(co);
    }
    
out:
    #ifdef STACK_SEPARATE
    STACK_OVERFLOW_CHECK(stack, SCH_STACK_SIZE);
    #else
    STACK_OVERFLOW_CHECK(current->getStack(), current->getStackSize());
    #endif

}

inline void Scheduler::timerProcess(){
    Coroutine *co = NULL;
    if(state != RUNNING || timerQue.empty()) goto out;
    
    co = timerQue.top();
    if(co->getTimeout() > time(NULL)) goto out;
    
    timerQue.pop();
    co->setState(RUNNABLE);
    co->setTimeout(0);
    addToRunQue(co);
    
out:
    #ifdef STACK_SEPARATE
    STACK_OVERFLOW_CHECK(stack, SCH_STACK_SIZE);
    #else
    STACK_OVERFLOW_CHECK(current->getStack(), current->getStackSize());
    #endif
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

    //never happen
    #ifdef STACK_SEPARATE
    STACK_OVERFLOW_CHECK(stack, SCH_STACK_SIZE);
    #else
    STACK_OVERFLOW_CHECK(current->getStack(), current->getStackSize());
    #endif
}

void Scheduler::stop(){
    state = STOPPED;
    
    while(true){
        Coroutine *co = CorMap::Instance()->next();
        if(co == NULL) return;
        co->stop();
    }
}

Scheduler::~Scheduler(){
    close(epfd);
    free(events);
    #ifdef STACK_SEPARATE
    free(stack);
    #endif
}

void addToRunQue(Coroutine *co){
    Scheduler::Instance()->addToRunQue(co);
}

void addToSigQue(Coroutine *co){
    Scheduler::Instance()->sigQue.push(co);
}

void addToTimerQue(Coroutine *co){
    Scheduler::Instance()->timerQue.push(co);
}

void cexit(int status){
    current->setState(DEAD);
    
    clear();
    switch(status){
        case -1:log(ERROR, "exit fail");break;
        case  0:log(INFO,  "exit sucess");break;
        
        default:log(INFO, "recv sig %d exit sucess", status);break;
    }
    log(INFO, "schedule to next");

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
    
    last = current;
    
    schedule();
}

void stopCoroutines(){
    Scheduler::Instance()->stop();
}

void startCoroutine(){
    cexit(current->routine(current->arg));
}
