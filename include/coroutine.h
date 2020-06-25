/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __COROUTINE__
#define __COROUTINE__

#include <time.h>
#include "context.h"

typedef int(*Routine)(void *);

class Coroutine{
private:
    friend void startCoroutine();
    friend struct compare;
    
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

    int erno;
    int signal;
    time_t timeout;
    
    //Coroutine() = delete;
    Coroutine(const Coroutine &) = delete;
    Coroutine &operator=(const Coroutine&) = delete;

public:
    Coroutine *next;
    
    Coroutine(int (*routine)(void *), void *arg);
	
    Coroutine();
	
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
        return id.fd;
    }

    int getSignal(){
        return signal;
    }

    void setSignal(int signo){
        if(!signo){
            signal = signo;
            return;
        }
        signal|=signo;
    }

    void setErno(int erno){
        this->erno = erno;
    }

    int getErno(){
        return erno;
    }

    void setTimeout(time_t timeout){
        this->timeout = timeout;
    }

    time_t getTimeout(){
        return timeout;
    }
    
    ~Coroutine();
};

extern __thread Coroutine *current;

Coroutine *createCoroutine(int (*routine)(void *),void *arg);

int getcid();
int gettid();

void setErno(int erno);
int getErno();

struct compare{
    bool operator()(Coroutine* &left, Coroutine* &right){
        return left->timeout > right->timeout;
    }
};

#endif
