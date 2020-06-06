/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __COROUTINE__
#define __COROUTINE__

#include "context.h"

typedef int(*Routine)(void *);

class Coroutine{
private:
    friend void startCoroutine();
    
    union Id{
        int fd;
        int cid;
    }id;
    int type;
    int epfd;

    void *arg;
    void *stack;
    int stackSize;
    Routine routine;
    Context context;
    
    int signal;

    Coroutine() = delete;
    Coroutine(const Coroutine &) = delete;
    Coroutine &operator=(const Coroutine&) = delete;

public:
    Coroutine *next;
    
    Coroutine(int (*routine)(void *), void *arg);
	
    Coroutine(int cid);
	
    int setStackSize(int size);

    int setContext(Context &cxt){
        context = cxt;
    }

    Context* getContext(){
        return &context;
    }

    void setFd(int fd){
        this->id.fd = fd;
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

    void setEpfd(int fd){
        epfd = fd;
    }

    int getEpfd(){
        return epfd;
    }
    
    void start();

    int getcid(){
        return id.cid;
    }

    int sendSignal(int signo){
        signal|=signo;
    }

    int getSignal(){
        return signal;
    }

    void setSignal(){
        signal = 0;
    }
    
    ~Coroutine();
};

extern __thread Coroutine *current;

void createCoroutine(int (*routine)(void *),void *arg);

int getcid();
int gettid();

#endif
