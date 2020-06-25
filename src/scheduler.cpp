/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#include <time.h>
#include "scheduler.h"
#include "csignal.h"

#define INTHZ 10

static __thread Coroutine *last = NULL;

__thread Scheduler* Scheduler::instance = NULL;

Scheduler::Scheduler(int max =1024){
    maxEventSize = max;
    epfd = epollCreate(max);
    events = (epoll_event*)malloc(max*sizeof(epoll_event));
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
        if(last != NULL){
            delete last;
            last = NULL;
        }        
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
    
    current = NULL;
    schedule();
}

void Scheduler::signalProcess(){
    int signal = current->getSignal();
    if(signal == 0) return;
    for(int signo=1; signo<32; signo++){
        if(signal & (1 << signo)){
            assert(signalHandler[signo] != NULL);
            signalHandler[signo](signo);
        }
    }
    current->setSignal(0);
}

inline void Scheduler::wakeup(){
    assert(current != NULL);
    signalProcess();
    restore(current->getContext());
}

void Scheduler::runProcess(){
    current = runQue.pop();
    if(current == NULL)
        return;
    
    if(getcid() != 0 || CorMap::Instance()->empty())
        wakeup();
    else
        runQue.push(current);
}

void Scheduler::eventProcess(){
    int firedEventSize = epollWait(epfd, events, maxEventSize, INTHZ);
    if(firedEventSize < 0)
        return;
    for(int nextEventIdx = 0; nextEventIdx < firedEventSize; nextEventIdx++){
        runQue.push((Coroutine*)events[nextEventIdx].data.ptr);
    }
}

inline void Scheduler::timerProcess(){
    if(timerQue.empty())
        return;
    
    Coroutine *co = timerQue.top();
    if(co->getTimeout() <= time(NULL)){
        timerQue.pop();
        co->setTimeout(0);
        runQue.push(co);
    }
}

int Scheduler::schedule(){
    while(true){
        runProcess();
        
        timerProcess();
        
        eventProcess();
    }
}

Scheduler::~Scheduler(){
    close(epfd);
    free(events);
}

void addToRunQue(Coroutine *co){
    Scheduler::Instance()->runQue.push(co);
}

void startCoroutine(){
    switch(current->routine(current->arg)){
        case -1:log(ERROR, "fd:%d exit fail", current->getFd());break;
        case  0:;
        case  1:log(INFO, "fd:%d exit sucess", current->getFd());break;
    }

    clear();
    last = current;
     
    log(INFO, "schedule to next");
    
    schedule();
}
