/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#include <sys/syscall.h>
#include "scheduler.h"
#include "csignal.h"
#include <time.h>
#include <stdlib.h>
#include "syscall.h"

#define INTHZ 10
#define LOADBALANCE_FACTOR 1

__thread Coroutine *last = NULL;

__thread Scheduler* Scheduler::instance = NULL;

Queue<Coroutine*, SpinLocker> Scheduler::RunQue;

volatile int Scheduler::Load = 0;
volatile int Scheduler::State = DEAD;
volatile int Scheduler::threads = 0;

Scheduler::Scheduler(int max =1024){
    idx = 0;
    State = NEW;
    running = 0;
    load = 0;
    
    maxEventSize = max;
    epfd = epollCreate(max);
    events = (epoll_event*)malloc(max*sizeof(epoll_event));
    
    groupid = syscall(__NR_gettid);
    
    #ifdef STACK_SEPARATE
    stack = (char*)malloc(SCH_STACK_SIZE);
    #endif
    __sync_fetch_and_add(&threads,1);
}

#define setEvent(co, epfd, fd, type)\
        do{\
            co->setFd(fd);\
            co->setType(type);\
            co->setEpfd(epfd);\
        }while(0)

int Scheduler::wait(int fd, int type, int timeout){
    if(fd > 0){
        current->setState(WAITING);
        if(current->getEpfd() < 0){
            CorMap::Instance()->del(current->getcid());
            setEvent(current, epfd, fd, type);
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
    
    schedule();
}

inline void Scheduler::wakeup(Coroutine *co){
    current = co;
    assert(co != NULL);
    assert(co->getState() != DEAD);
    assert(co->getState() != RUNNING);
    
    if(getcid() == 0) State = DEAD;
    else if(State == RUNNING) State = RUNNABLE;
    
    co->setState(RUNNING);
    restore(co->getContext(), (long)doSignal);
}

Coroutine* Scheduler::next(){
    Coroutine *co = NULL;
    
    for(; idx < SCH_PRIO_SIZE; idx++){
        if(runQue[running][idx].empty()) continue;
        co = runQue[running][idx].pop();
        runables--;
        return co;
    }
    idx = 0;
    running = !running;
    
    return co;
}

void Scheduler::relax(){
    if(State != RUNNING || CorMap::Instance()->empty()) return;

    int half = runables / 2;
    for(; half > 0 ; half--){
        Coroutine *co = next();
        if(co == NULL) break;
        
        if(co->getcid() == 0){
            addToRunQue(co);
            continue;
        }
        
        unbind(co);
        RunQue.push(co);
        log(INFO,"relax %d %d %d",co->getcid(),load, Load);
    }
    
    if(!half) load = load / 2;
}

inline Coroutine* Scheduler::stress(){
    if(RunQue.empty()) return NULL;
    
    Coroutine *co = RunQue.pop();
    if(co == NULL) return NULL;
    
    bind(co);
    return co;
}

inline Coroutine *Scheduler::loadbalance(){
    if(threads > 0 && State == RUNNING && load > LOADBALANCE_FACTOR && Load < LOADBALANCE_FACTOR)
        relax();
    
    Coroutine *co = next();
    if(co == NULL && ( load < LOADBALANCE_FACTOR || State != RUNNING || threads == 1))
        return stress();
    
    return co;
}

inline void Scheduler::runProcess(){
    
    Coroutine *co = loadbalance();
    if(co == NULL) return;

    
    if(co->getcid() != 0 ||
        CorMap::Instance()->empty()){
        wakeup(co);
    }else{
        addToRunQue(co);
    }
    
    return;
}

inline void Scheduler::timerProcess(){
    Coroutine *co = NULL;
    if(State != RUNNING || timerQue.empty())
        return;
    
    co = timerQue.top();
    if(co->getTimeout() > time(NULL))
        return;
    
    timerQue.pop();
    co->setState(RUNNABLE);
    co->setTimeout(0);
    addToRunQue(co);
    
    return;
}

void Scheduler::eventProcess(){
    if(State != RUNNING) return;
    
    int firedEventSize = epollWait(epfd,
                                   events,
                                   maxEventSize,
                                   INTHZ);
    if(firedEventSize == 0){
        if(load > 0) load--;
        if(Load > 0) Load--;
    }else if(firedEventSize == maxEventSize){
        load++;
        Load++;
    }

    int nextEventIdx = 0;
    for(; nextEventIdx < firedEventSize; nextEventIdx++){
        Coroutine *co = NULL;
        co = (Coroutine*)events[nextEventIdx].data.ptr;
        assert(co->getState() != DEAD);
        co->setState(RUNNABLE);
        addToRunQue(co);
    }
    
    return;
}

inline void Scheduler::signalProcess(){
    //在多线程环境下，sigQue为非线程安全，在异步信号处理函数中调用stop()操作sigQue时要阻止其它线程同时操作sigQue
    if(State == STOPPING) return;
    
    Coroutine *co = sigQue.pop();
    if(co == NULL) return;
    
    assert(co->getState() != DEAD);
    co->setIntr(true);
    co->setState(RUNNABLE);
    addToRunQue(co);
    return;
}

void Scheduler::schedule(){
    if(State == RUNNABLE || State == NEW) State = RUNNING;
    
    while(true){
        runProcess();
        timerProcess();
        eventProcess();
        signalProcess();
    }
    
}

void Scheduler::stop(){
    State = STOPPING;

    //处于全局共享队列中的协程还没有绑定到调度器线程，等待调度器线程调度绑定这些协程
    if(!RunQue.empty()){
        log(WARN, "can`t stop immediately, the global RunQue size is %d, please stop it again late", RunQue.size());
        return;
    }

    //暂停一下尽可能等待其它调度器 线程检测到State == STOPPING
    sysSleep(1);
    
    while(true){
        Coroutine *co = CorMap::Instance()->next();
        if(co == NULL) break;
        co->stop();
    }
    State = STOPPED;
}

Scheduler::~Scheduler(){
    close(epfd);
    free(events);
    #ifdef STACK_SEPARATE
    free(stack);
    #endif
    __sync_fetch_and_sub(&threads,1);
}

void addToRunQue(Coroutine *co){
    Scheduler::Instance()->RunQue.push(co);
    return;
}

int waitOnTimer(int timeout){
    return(wait(-1, -1, timeout));
}

void wakeup(Coroutine *co){
    Scheduler::Instance()->wakeup(co);
    return;
}

void stopCoroutines(){
    Scheduler::Instance()->stop();
    return;
}

void cexit(int status){
    
    current->setState(DEAD);
    clear();
    switch(status){
        case -1:
        case  0:
        default:break;
    }
    
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
