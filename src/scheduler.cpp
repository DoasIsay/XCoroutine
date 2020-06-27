/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#include <time.h>
#include <sys/syscall.h>
#include "scheduler.h"
#include "csignal.h"

#define INTHZ 10

__thread Scheduler* Scheduler::instance = NULL;

Queue<Coroutine*, SpinLocker> Scheduler::sigQue;

volatile int Scheduler::status = Scheduler::STOPED;

Scheduler::Scheduler(int max =1024){
    status = RUNNING;
    
    maxEventSize = max;
    epfd = epollCreate(max);
    events = (epoll_event*)malloc(max*sizeof(epoll_event));

    groupid = syscall(__NR_gettid);
    
    stack = (char*)malloc(STACKSIZE) + STACKSIZE;
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
        return 1;
    }
    
    if(fd > 0){
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
        runQue.push(current);
    }
    
    if(timeout > 0){
        current->setTimeout(timeout);
        timerQue.push(current);
    }
    
    schedule();
}

void Scheduler::doSignal(){
    int signal = current->getSignal();
    if(signal == 0) return;
    
    for(int signo=1; signo<32; signo++){
        if(!(signal & (1 << signo))) continue;
        assert(signalHandler[signo] != NULL);
        signalHandler[signo](signo);
    }
    
    current->setSignal(0);
}

void Scheduler::signalProcess(){
    Coroutine *co = sigQue.pop();
    if(co == NULL) return;
    
    if(co->getGroupid() == getGrouid())
        runQue.push(co);
    else
        sigQue.push(co);
}

inline void Scheduler::wakeup(){
    assert(current != NULL);
    doSignal();
    restore(current->getContext());
}

void Scheduler::runProcess(){
    Coroutine *next= runQue.pop();
    if(next == NULL) return;

    current = next;
    if(getcid() != 0 || CorMap::Instance()->empty())
        wakeup();
    else
        runQue.push(current);
}

void Scheduler::eventProcess(){
    if(status == STOPED) return;
    
    int firedEventSize = epollWait(epfd, events, maxEventSize, INTHZ);
    for(int nextEventIdx = 0; nextEventIdx < firedEventSize; nextEventIdx++){
        runQue.push((Coroutine*)events[nextEventIdx].data.ptr);
    }
}

inline void Scheduler::timerProcess(){
    if(status == STOPED || timerQue.empty()) 
        return;
    
    Coroutine *co = timerQue.top();
    if(co->getTimeout() <= time(NULL)){
        timerQue.pop();
        co->setTimeout(0);
        runQue.push(co);
    }
}

void Scheduler::schedule(){
    while(true){
        runProcess();
        
        timerProcess();
        
        eventProcess();

        signalProcess();
    }
}

void Scheduler::stop(){
    status = STOPED;
    
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
    free(events);
}

void addToRunQue(Coroutine *co){
    Scheduler::Instance()->runQue.push(co);
}

void addToSigQue(Coroutine *co){
    Scheduler::Instance()->sigQue.push(co);
}

void addToTimerQue(Coroutine *co){
    Scheduler::Instance()->timerQue.push(co);
}

void cexit(int status){
    clear();
    switch(status){
        case -1:log(ERROR, "exit fail");break;
        case  0:log(INFO,  "exit sucess");break;
        
        default:log(INFO, "recv sig %d exit sucess", status);break;
    }
    log(INFO, "schedule to next");
 
    char *schedStack= Scheduler::Instance()->getStack();
    assert(schedStack != NULL);
    
    //switch coroutine stack to scheduler stack
    #ifdef __i386__
        asm("movl %0,%%esp;"::"m"(schedStack));
    #elif __x86_64__
        asm("movq %0,%%rsp;"::"m"(schedStack));
    #endif

    //now we can delete current coroutine safely on scheduler stack
    delete current;
    current = NULL;
    
    schedule();
}

void stopCoroutines(){
    Scheduler::Instance()->stop();
}

void startCoroutine(){
    cexit(current->routine(current->arg));
}
