/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */
 
#include "csignal.h"

SignalHandler signalHandler[32] = {defaultHandler};

extern __thread std::unordered_map<int, Coroutine*> *corMap;

int ckill(int cid, char signo){
    std::unordered_map<int, Coroutine*>::iterator findIt = corMap->find(cid);
    if(findIt != corMap->end()){
        if(signo == 0) return 0;
        
        findIt->second->sendSignal(signo);
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