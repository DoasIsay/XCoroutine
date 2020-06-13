/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#include <assert.h>
#include "coroutine.h"
#include "csignal.h"
#include "cormap.h"

SignalHandler signalHandler[32] = {defaultHandler};

int ckill(int cid, char signo){
    Coroutine *co = CorMap::Instance()->get(cid);
    if(co != NULL){
        if(signo == 0) return 0;
        
        co->setSignal(signo);
        return 0;
    }else
        return -1;
}

int csignal(int signo, SignalHandler handler){
    assert(signo < 32 && signo >= 0);
    signalHandler[signo] = handler;
}

void defaultHandler(int signo){

}