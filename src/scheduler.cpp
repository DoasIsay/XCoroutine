/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#include "scheduler.h"
#include "csignal.h"
#include "cormap.h"

#define INTHZ 10

__thread Scheduler* Scheduler::instance = NULL;

Scheduler::Scheduler(int max =1024){
    nextEventIdx = 0;
    firedEventSize = 0;
    maxEventSize = max;
    epfd = epollCreate(max);
    events = (epoll_event*)malloc(max*sizeof(epoll_event));
}

inline Coroutine* Scheduler::next(){
    if(nextEventIdx < firedEventSize){
        return (Coroutine*)(events[nextEventIdx++].data.ptr);
    }
    nextEventIdx=firedEventSize=0;
    return NULL;
}

#define setEvent(epfd, fd, type)\
    do{\
        current->setFd(fd);\
        current->setType(type);\
        current->setEpfd(epfd);\
    }while(0)

int Scheduler::wait(int fd, int type){
    assert(current != NULL);
    if(save(current->getContext()))
        return 1;
    
    if(fd > 0){
        if(current->getcid() >= CorMap::STARTCID){
            CorMap::Instance()->del(current->getcid());
            setEvent(epfd, fd, type);
            epollAddEvent(epollFd, fd, type);
            CorMap::Instance()->set(fd, current);
        }
        else if(current->getType() != type){
            setEvent(epfd, fd, type);
            epollModEvent(epfd, fd, type);
        }
    }else{
        addToRunQue(current);
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
    current->setSignal();
}

inline void Scheduler::wakeup(){
    signalProcess();
    restore(current->getContext());
}

inline void Scheduler::timerInterrupt(){
    
    if(!runQue.empty()){
        current = delFromRunQue();
        if(getcid() != 0 || CorMap::Instance()->empty()){
            wakeup();
        }
        else{
            addToRunQue(current);
            current = NULL;
        }
    }
}

int Scheduler::schedule(){
    while(true){
        if((current = next()) != NULL) break;
        firedEventSize =  epollWait(epollFd, events, maxEventSize, INTHZ);
        if(firedEventSize == 0)
            timerInterrupt();
    }
    wakeup();
}

Scheduler::~Scheduler(){
    close(epollFd);
    free(events);
}

void addToRunQue(Coroutine *co){
    Scheduler::Instance()->addToRunQue(co);
}

void startCoroutine(){
    switch(current->routine(current->arg)){
        case -1:log(ERROR, "fd %d exit fail", current->id.fd);break;
        case  0:
        case  1:log(INFO, "fd %d exit sucess", current->id.fd);break;
    }

    if(current != NULL){
        if(current->id.fd < CorMap::STARTCID){
            epollDelEvent(current->epfd, current->id.fd, current->type);
        }
        delete current;
        current = NULL;
    }
    
    log(INFO, "schedule to next");
    schedule();
}
