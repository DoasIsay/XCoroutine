/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

//线程间同步锁

#ifndef __LOCKER__
#define __LOCKER__

#include<pthread.h>

class Locker{
public:
    virtual void lock(){}
    virtual void unlock(){}
    virtual bool trylock(){}
    virtual bool state(){}
    virtual ~Locker(){}
};

class MutexLocker:public Locker{
private:
    pthread_mutex_t mutex;

public:
    MutexLocker(){
        pthread_mutex_init(&mutex, NULL);
    }
        
    ~MutexLocker(){
        pthread_mutex_destroy(&mutex);
    }

    void lock(){
        pthread_mutex_lock(&mutex);
    }

    void unlock(){
        pthread_mutex_unlock(&mutex);
    }

    bool trylock(){
        return pthread_mutex_trylock(&mutex);
    }

    bool state(){
        return !trylock();
    }
};

class SpinLocker:public Locker{
private:
    volatile int flag;

public:
    SpinLocker():flag(0){}

    void lock(){
        while(__sync_lock_test_and_set(&flag, 1)){
            asm volatile("rep; nop");
        }
    }

    void unlock(){
        __sync_lock_release(&flag);
    }

    bool trylock(){
        return __sync_lock_test_and_set(&flag,1) == 0;
    }

    bool state(){
        return flag;
    }
};

#endif
