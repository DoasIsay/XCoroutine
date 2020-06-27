/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#include <assert.h>
#include "csignal.h"
#include "cormap.h"

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
    co->setSignal(signo);
    addToSigQue(co);
}

int ckill(int cid, int signo){
    Coroutine *co = CorMap::Instance()->get(cid);
    if(co != NULL){
        ckill(co, signo);
        return 0;
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
