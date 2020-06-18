#ifndef __LOCKER__
#define __LOCKER__

#include<pthread.h>

class Locker
{
public:
    virtual void lock(){}
    virtual void unlock(){}
};

class MutexLocker:public Locker
{
private:
    pthread_mutex_t mutex;

public:
    MutexLocker()
    {
        pthread_mutex_init(&mutex,NULL);
    }
        
    ~MutexLocker()
    {
        pthread_mutex_destroy(&mutex);
    }

    void lock()
    {
        pthread_mutex_lock(&mutex);
    }

    void unlock()
    {
        pthread_mutex_unlock(&mutex);
    }
};

class SpinLocker:public Locker
{
private:
    volatile int flag;

public:
    SpinLocker():flag(0){}

    void lock()
    {
        while(__sync_lock_test_and_set(&flag,1)){
            asm volatile("rep; nop");
        }
    }

    void unlock()
    {
        __sync_lock_release(&flag);
    }
};

#endif
