/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __COROUTINE__
#define __COROUTINE__

#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include "context.h"


enum{
    NEW,
    RUNNABLE,
    RUNNING,
    WAITING,
    STOPPED,
    DEAD
};

typedef int(*Routine)(void *);

class Coroutine{
public:
    #ifdef STACK_CHECK
    long lowMagic;
    #endif
    
private:
    friend void startCoroutine();
    friend struct compare;
    
    int state;
    int prio;
    
    int type;
    int epfd;
    
    union Id{
        int fd;
        int cid;
    }id;
    pid_t groupid;
    
    void *arg;
    char *stack;
    int stackSize;
    Routine routine;
    Context context;
    
    int signal;
    bool  intr;
    time_t timeout;
    
public:
    #ifdef STACK_CHECK
    long highMagic;
    #endif
    
private:
    Coroutine(const Coroutine &) = delete;
    Coroutine &operator=(const Coroutine&) = delete;

private:
    void init(Routine routine, void *arg, int prio, int cid);
    
public:
    Coroutine *next;
    
    Coroutine(Routine routine, void *arg);
	
    Coroutine();
	
    void setStackSize(int size){
        this->stackSize = size;
    }

    int getStackSize(){
        return stackSize;
    }
    
    char* getStack(){
        return stack;
    }
    
    int setContext(Context &cxt){
        context = cxt;
    }

    Context* getContext(){
        return &context;
    }

    void setFd(int fd){
        id.fd = fd;
    }
    
    int getFd(){
        return id.fd;
    }

    void setType(int type){
        this->type = type;
    }
    
    int getType(){
        return type;
    }

    void setEpfd(int epfd){
        this->epfd = epfd;
    }

    int getEpfd(){
        return epfd;
    }
    
    int start();

    void stop();
    
    int getcid(){
        return id.cid;
    }

    pid_t getGroupid(){
        return groupid;
    }
    
    int getSignal(){
        return signal;
    }

    void setSignal(int signo){
        if(signo == 0){
            signal = 0;
            return;
        }
        signal |= (1<<signo);
    }
    
    void setTimeout(time_t timeout){
        this->timeout = timeout;
    }

    time_t getTimeout(){
        return timeout;
    }

    void setState(int state){
        this->state = state;
    }

    int getState(){
        return state;
    }

    void setIntr(bool intr){
        this->intr = intr;
    }

    bool getIntr(){
        return intr;
    }
    
    void setPrio(int prio){
        assert(prio<=SCH_PRIO_SIZE && prio>=1);
        this->prio = prio - 1;
    }
    
    int getPrio(){
        return prio;
    }
    ~Coroutine();
};

struct compare{
    bool operator()(Coroutine* &left, Coroutine* &right){
        return left->timeout > right->timeout;
    }
};

int getcid();
int gettid();

extern __thread Coroutine *current;

extern int waitOnTimer(int timeout);
extern void wakeup(Coroutine *co);
extern void stopCoroutines();

Coroutine *createCoroutine(int (*routine)(void *), void *arg);

#define yield\
	do{\
		if(current == NULL){\
			current = new Coroutine();\
			current->start();\
		}\
		waitOnTimer(0);\
	}while(0)

#define resume(co)\
	do{\
		if(co != NULL){\
            wakeup(co)\
		}\
	}while(0)

#endif
