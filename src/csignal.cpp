/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#include <assert.h>
#include "csignal.h"
#include "cormap.h"
#include "scheduler.h"

SignalHandler signalHandler[32] = {sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler,
                                   sigdefHandler
                                   };

extern void addToSigQue(Coroutine * co);

int ckill(Coroutine *co, int signo){
    assert(co != NULL);
    assert(co->getState() != DEAD);
    
    if(signo == 0) return 0;
    
    co->setSignal(signo);
    //在cruuent协程上下文中发生信号处理，当从信号处理函数返回后current协程可能会立即退出
    if(getcid() == co->getcid()) return 0;

    assert(co->getSched() != NULL);
    co->getSched()->addToSigQue(co);
    return 0;
}

int ckill(int cid, int signo){
    Coroutine *co = CorMap::Instance()->get(cid);
    if(co != NULL){
        if(signo == 0) return 0;
        return ckill(co, signo);
    }else
        return -1;
}

int csignal(int signo, SignalHandler handler){
    assert(signo < 32 && signo >= 0);
    signalHandler[signo] = handler;
}

extern void cexit(int status);

void sigdefHandler(int signo){
    cexit(signo);
}

void doSignal(){
    int signal = current->getSignal();
    if(signal == 0) return;
    
    for(int signo=1; signo<32; signo++){
        if(!(signal & (1 << signo))) continue;
        assert(signalHandler[signo] != NULL);
        signalHandler[signo](signo);
    }
    current->setSignal(0);
    return;
}

