#ifndef __COUNT_DOWN_LATCH__
#define __COUNT_DOWN_LATCH__

#include "cond.h"
#include "mutex.h"

class CountDownLatch{
private:
    volatile int count;
    Cond cond;
    Mutex mutex;

public:
    CountDownLatch(int count){
        this->count = count;
    }

    int wait(int timeout = -1){
        if(count == 0) return 0;
        
        mutex.lock();
        if(count == 0){
            mutex.unlock();
            return 0;
        }
        int ret = cond.wait(mutex, timeout);
        mutex.unlock();

        return ret;
    }

    void countDown(){
        mutex.lock();
        count--;
        mutex.unlock();

        if(count == 0) cond.signal();
    }

    int getCount(){
        return count;
    }
};

#endif
