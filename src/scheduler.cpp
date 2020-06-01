/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#include "scheduler.h"
#include "csignal.h"
#include "log.h"

#define INTHZ 10

Scheduler::Scheduler(int max =1024){
    nextEventIdx = 0;
    firedEventSize = 0;
    maxEventSize = max;
    epollFd = epollCreate(max);
    events = (epoll_event*)malloc(max*sizeof(epoll_event));
}

inline Coroutine* Scheduler::next(){
    if(nextEventIdx < firedEventSize){
        return (Coroutine*)(events[nextEventIdx++].data.ptr);
    }
    nextEventIdx=firedEventSize=0;
    return NULL;
}

#define setEvent(epollFd, fd, type)\
    do{\
        current->setFd(fd);\
        current->setType(type);\
        current->setEpollFd(epollFd);\
    }while(0)

int Scheduler::wait(int fd, int type){
    assert(current != NULL);
    if(save(current->getContext()))
        return 1;
    
    if(fd > 0){
        if(current->getFd() < 0){
            setEvent(epollFd, fd, type);
            epollAddEvent(epollFd, fd, type);
        }else if(current->getType() != type){
            setEvent(epollFd, fd, type);
            epollModEvent(epollFd, fd, type);
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
        if(current->getcid()!=0 || (corMap->size() == 1))
            wakeup();
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

__thread Scheduler *scheduler = NULL;

void addToRunQue(Coroutine *co){
    scheduler->addToRunQue(co);
}

void startCoroutine(){
    switch(current->routine(current->arg)){
        case -1:log(ERROR, "fd %d exit fail", current->fd);break;
        case  0:
        case  1:log(INFO, "fd %d exit sucess", current->fd);break;
    }

    if(current != NULL){
        epollDelEvent(current->epollFd, current->fd, current->type);
        delete current;
        current = NULL;
    }
    
    log(INFO, "schedule to next");
    schedule();
}

class Enviroment{
private:
    static int times;
public:
    Enviroment(){
        times++;
        if(scheduler == NULL){
            scheduler = new Scheduler(1024);
        }
       
        if(corMap == NULL){
            corMap = new std::unordered_map<int, Coroutine*>;
        }
        log(INFO, "env initialize %d\n", times);
    }

    ~Enviroment(){
        if(scheduler != NULL){
             delete scheduler;
         }

         if(corMap != NULL){
             delete corMap;
             corMap = NULL;
         }
         log(INFO, "env destory %d\n", times);
         times--;
    }
};

int Enviroment::times = 0;

Enviroment env;


