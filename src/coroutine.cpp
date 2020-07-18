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
#include "coroutine.h"
#include "cormap.h"
#include "memory.h"

const static int DEF_PRIO = SCH_PRIO_SIZE/2;

__thread Coroutine *current = NULL;

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

void Coroutine::init(Routine routine, void *arg,
        int prio, int cid){
    type = -1;
    epfd = -1;
    signal = 0;
    timeout = 0;
    next = this;
    intr = false;
    
    stack = NULL;
    this->arg = arg;
    stackSize = COR_STACK_SIZE;
    this->routine = routine;
    
    id.cid = cid;
    groupid = syscall(__NR_gettid);
    setState(NEW);
    setPrio(prio);
    
    #ifdef STACK_CHECK
    lowMagic = highMagic = 0x0123456789abcde0;
    #endif
}


Coroutine::Coroutine(Routine routine, void *arg){
    init(routine, arg, DEF_PRIO, allocCid(this));
}

Coroutine::Coroutine(){
    init(NULL, NULL ,SCH_PRIO_SIZE, 0);
}

extern void startCoroutine();
extern void addToRunQue(Coroutine *co);
extern int  ckill(Coroutine *co, int signo);

int Coroutine::start(){
    if(getcid() < 0) return -1;
    
    save(&context);
    context.pc = (long)startCoroutine;

    if(stack == NULL) stack = allocMem(stackSize);
    assert(stack != NULL);

    #ifdef STACK_CHECK
    context.sp = (long)(stack + stackSize - 8);
    #else
    context.sp = (long)(stack + stackSize);
    #endif
    
    setState(RUNNABLE);
    addToRunQue(this);
    return 0;
}

void Coroutine::stop(){
    ckill(this, SIGTERM);
    setState(STOPPED);
}

Coroutine::~Coroutine(){
    STACK_OVERFLOW_CHECK;
    if(stack != NULL) freeMem(stack, stackSize);
}

Coroutine *createCoroutine(int (*routine)(void *), void *arg){
    Coroutine *co= new Coroutine(routine,arg);
    co->start();
    return co;
}

int getcid(){
    if(current == NULL) return 0;
    return current->getcid();
}

int gettid(){
    if(current == NULL) return syscall(__NR_gettid);
    return current->getGroupid();
}
