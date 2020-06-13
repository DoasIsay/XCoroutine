/*
 * Copyright (c) 2020, wenwu xie <870585356@qq.com>
 * All rights reserved.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <assert.h>
#include "log.h"
#include "coroutine.h"
#include "cormap.h"

extern void addToRunQue(Coroutine *co);
extern void startCoroutine();

__thread Coroutine *current = NULL;

const static int STACKSIZE = 8192;

static int allocCid(Coroutine *co){
    int cid;
    CorMap *corMap = CorMap::Instance();
    static SpinLocker locker;
    locker.lock();
    for(cid = CorMap::STARTCID ; corMap->get(cid) != NULL; ++cid);
    CorMap::Instance()->set(cid, co);
    locker.unlock();
    return cid; 
}

Coroutine::Coroutine(int (*routine)(void *), void *arg){
    type = -1;
    epfd = -1;
    signal = 0;
    next = NULL;
    
    stack = NULL;
    this->arg = arg;
    stackSize = STACKSIZE;
    this->routine = routine;
    
    id.cid = allocCid(this);
}

Coroutine::Coroutine(int cid){
    type = -1;
    epfd = -1;
    signal = 0;
    next = NULL;
    this->id.cid = cid;
}

int Coroutine::setStackSize(int size){
    this->stackSize = size;
}

void Coroutine::start(){
    if(getcid() < 0)
        return;
    save(&context);
    context.pc = (long)startCoroutine;
    
    stack = malloc(stackSize);
    assert(stack != NULL);
    
    context.sp = (long)((char*)stack + stackSize);   
    addToRunQue(this);
}

Coroutine::~Coroutine(){
    if(stack != NULL)
        free(stack);
    CorMap::Instance()->del(getcid());
}

Coroutine *createCoroutine(int (*routine)(void *),void *arg){
    Coroutine *co= new Coroutine(routine,arg);
    co->start();
    return co;
}

int getcid(){
    if(current == NULL)
        return 0;
    return current->getcid();
}
int gettid(){
    return syscall(__NR_gettid);
}

int getErno(){
    if(current == NULL)
        return 0;
    return current->getErno();
}

void setErno(int erno){
    if(current == NULL)
        return;
    current->setErno(erno);
}

