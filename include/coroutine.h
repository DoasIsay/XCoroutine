/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __COROUTINE__
#define __COROUTINE__
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "context.h"
#include <set>
#include <assert.h>
#include <unordered_map>
#include "log.h"


typedef int(*Routine)(void *);

class Coroutine{
private:
    friend void startCoroutine();
    int epollFd;
    int fd;
    int type;
    
    Routine routine;
    void *stack;
    int stackSize;
    void *arg;
    Context context;
    int cid;
    
    int signal;

public:
    
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
        this->fd = fd;
    }
    int getFd(){
        return fd;
    }

    void setType(int type){
        this->type = type;
    }
    
    int getType(){
        return type;
    }

    void setEpollFd(int eFd){
        epollFd = eFd;
    }

    int getEpollFd(){
        return epollFd;
    }
    
    void start();

    int getcid(){
        return cid;
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

extern __thread std::unordered_map<int, Coroutine*> *corMap;

void createCoroutine(int (*routine)(void *),void *arg);

int getcid();
int gettid();
#endif
