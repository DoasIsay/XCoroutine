/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __MUTEX__
#define __MUTEX__

#include "queue.h"
#include "log.h"

class Coroutine;
extern __thread Coroutine *current;
extern int waitOnTimer(int timeout);

extern int getcid();
extern int gettid();

class Mutex:public Locker{
private:
    SpinLocker locker;
    volatile int ownCid;
    volatile int ownTid;
    
    SpinLocker gaurd;
    Queue<Coroutine*> waitQue;

public:
    void lock(){
        if(ownCid == getcid()) return;
        
        while(!trylock()){
            /*gaurd用来保证 1.trylock获取锁失败 2.push到waitQue的两个操作的原子性，同时保护waitQue
              可防止在另一个线程中的协程unlock发生在第2步操作前
              如果另一个线程中的协程在第2步操作前就unlock，会导致当前协程无法被唤醒产生死锁
             */
            gaurd.lock();
            if(trylock()){
                gaurd.unlock();
                goto out;
            }
            /*不能在waitOnTimer中修改协程的状态，此协程有可能被另一个线程中的协程signal唤醒
              导致同一个协程被多个线程调度，所以应在push到waitQue前就修改状态
             */
            current->setState(SYNCING);
            waitQue.push(current);
            log(INFO, "lock fail ownCid %d ownTid %d, wait for it", ownCid, ownTid);
            gaurd.unlock();  
            
            if(waitOnTimer(-1) < 0)//被信号中断返回 
                return;
        }
    out:
        ownCid = getcid();
        ownTid = gettid();
    }
    
    void unlock(){
        gaurd.lock();
        
        Coroutine *co = waitQue.pop();
        locker.unlock();
        ownCid = 0;
        ownTid = 0;

        gaurd.unlock();
        if(co == NULL) return;
        wakeup(co);
        log(INFO, "unlock wakeup %d", co->getcid());
    }

    bool trylock(){
        return locker.trylock();
    }

    bool state(){
        return locker.state();
    }

    int getOwnCid(){
        return ownCid;
    }

    int getOwnTid(){
        return ownTid;
    }
};

#endif
