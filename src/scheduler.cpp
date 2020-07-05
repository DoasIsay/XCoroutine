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
    
    stack = (char*)malloc(SCH_STACK_SIZE);
}

#define setEvent(epfd, fd, type)\
        do{\
            current->setFd(fd);\
            current->setType(type);\
            current->setEpfd(epfd);\
        }while(0)

int Scheduler::wait(int fd, int type, int timeout){
    assert(current != NULL);
    if(save(current->getContext())){
        if(current->getIntr()){
            current->setIntr(false);
            return -1;
        }
        return 1;
    }
    
    if(fd > 0){
        current->setState(WAITING);
        if(current->getEpfd() < 0){
            CorMap::Instance()->del(current->getcid());
            setEvent(epfd, fd, type);
            epollAddEvent(epfd, fd, type);
            CorMap::Instance()->set(fd, current);
            
        }
        else if(current->getType() != type){
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
    
    STACK_OVERFLOW_CHECK(current->getStack());
    
    current = NULL;
    schedule();
}

void Scheduler::doSignal(){
    int signal = current->getSignal();
    if(signal == 0) return;
    
    for(int signo=1; signo<32; signo++){
        if(!(signal & (1 << signo))) continue;
        assert(signalHandler[signo] != NULL);
        log(INFO,"signal %d %d",signal,current->getSignal());
        signalHandler[signo](signo);
    }
    
    current->setSignal(0);

    STACK_OVERFLOW_CHECK(stack);
}

void Scheduler::signalProcess(){
    Coroutine *co = sigQue.pop();
    if(co == NULL) return;
    
    if(co->getGroupid() == groupid){
        if(co->getState() == WAITING) co->setIntr(true);
        co->setState(RUNNABLE);
        assert(co->getGroupid()!=0);
        addToRunQue(co);
    }else{
        sigQue.push(co);
    }
    
    STACK_OVERFLOW_CHECK(stack);
}

inline void Scheduler::wakeup(){
    assert(current != NULL);
    assert(current->getState() != DEAD);
    
    if(getcid() == 0)
        state = DEAD;
    else if(state != STOPPED)
        state = RUNNABLE;
    
    doSignal();
    current->setState(RUNNING);
    
    STACK_OVERFLOW_CHECK(stack);
    restore(current->getContext());
}

Coroutine* Scheduler::next(){
    for(; idx < SCH_PRIO_SIZE; idx++){
        if(runQue[running][idx].empty())
            continue;
        return runQue[running][idx].pop();
    }

    idx = 0;
    running = !running;

    STACK_OVERFLOW_CHECK(stack);
    
    return NULL;
}
void Scheduler::runProcess(){
    Coroutine *co= next();
    if(co == NULL) return;

    current = co;
    if(getcid() != 0 ||
        CorMap::Instance()->empty())
        wakeup();
    else{
        current = NULL;
        addToRunQue(co);
    }
    
    STACK_OVERFLOW_CHECK(stack);
}

void Scheduler::eventProcess(){
    if(state == STOPPED) return;
    
    int firedEventSize = epollWait(epfd, events, maxEventSize, INTHZ);
    for(int nextEventIdx = 0; nextEventIdx < firedEventSize; nextEventIdx++){
        Coroutine *co = (Coroutine*)events[nextEventIdx].data.ptr;
        assert(co->getState() != DEAD);
        co->setState(RUNNABLE);
        addToRunQue(co);
    }

    STACK_OVERFLOW_CHECK(stack);
}

inline void Scheduler::timerProcess(){
    if(state != RUNNING || timerQue.empty())
        return;
    
    Coroutine *co = timerQue.top();
    if(co->getTimeout() <= time(NULL)){
        timerQue.pop();
        co->setState(RUNNABLE);
        co->setTimeout(0);
        addToRunQue(co);
    }

    STACK_OVERFLOW_CHECK(stack);
}

void Scheduler::schedule(){
    long sp = (long)(getStack() + SCH_STACK_SIZE);
    
    //switch coroutine stack to scheduler stack
    #ifdef __i386__
        asm("movl %0,%%esp;"::"m"(sp));
    #elif __x86_64__
        asm("movq %0,%%rsp;"::"m"(sp));
    #endif
    
    //now we can delete current coroutine safely on scheduler stack
    if(current != NULL){
        delete current;
        current = NULL;
    }

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
        if(co != NULL)
            co->stop();
        else
            return;
    }
}

Scheduler::~Scheduler(){
    close(epfd);
    free(stack);
    free(events);
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
    
    schedule();
}

void stopCoroutines(){
    Scheduler::Instance()->stop();
}

void startCoroutine(){
    cexit(current->routine(current->arg));
}
