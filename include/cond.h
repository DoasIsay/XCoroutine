/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __COND__
#define __COND__

#include "mutex.h"

class Cond{
public:
    Queue<Coroutine*, SpinLocker> waitQue;
    
    void signal(){
        Coroutine *co = waitQue.pop();
        if(co != NULL){
            wakeup(co);
            log(INFO, "signal %d", co->getcid());
        }
    }
    
    void broadcast(){
        while(!waitQue.empty()){
            Coroutine *co = waitQue.pop();
            if(co != NULL)
            wakeup(co);
        }    
    }
    
    int wait(Mutex &mutex, int timeout = -1){
        waitQue.push(current);
        
        mutex.unlock();
        int ret = waitOnTimer(timeout);
        mutex.lock();

        return ret;
    }
};

#endif
