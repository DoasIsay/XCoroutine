/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __SEM__
#define __SEM__

#include "cond.h"

class Sem{
private:
    volatile int permits, used; 
    Mutex mutex;
    Cond cond;
    
public:
    Sem(int pers){
        used = 0;
        permits = pers;
    }
    
    void wait(){
        mutex.lock();
        
        while(used >= permits){
           log(INFO, "permits %d used %d, wait until used < permits", permits, used);
           cond.wait(mutex);
           log(INFO, "wakeup used %d", used);
        }
        used++;
            
        mutex.unlock();
    }

    void signal(){
        mutex.lock();
        if(used > 0) used--;
        mutex.unlock();

        cond.signal();
    }
};

#endif
