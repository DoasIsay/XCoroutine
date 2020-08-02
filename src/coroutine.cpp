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
    corMap ->set(cid, co);
    locker.unlock();
    return cid; 
}
extern Queue<Coroutine*> *getSigQue();
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
    groupid = 0;
    
    state = NEW;
    setPrio(prio);
    schdeuler = NULL;
}


Coroutine::Coroutine(Routine routine, void *arg){
    init(routine, arg, DEF_PRIO, allocCid(this));
}

Coroutine::Coroutine(){
    init(NULL, NULL ,SCH_PRIO_SIZE, 0);
}

extern void startCoroutine();
extern void addToRunQue(Coroutine *co);

int Coroutine::start(){
    if(getcid() < 0) return -1;
    
    save(&context);
    context.pc = (long)startCoroutine;
    
    if(stack == NULL) stack = (char*)malloc(stackSize);
    assert(stack != NULL);
    context.sp = (long)(stack + stackSize);
    
    setState(RUNNABLE);
    addToRunQue(this);
    return 0;
}

void Coroutine::stop(){
    if(state == DEAD) return;
    setState(STOPPING);
    ckill(this,SIGTERM);
}

Coroutine::~Coroutine(){
    setState(DEAD);
    assert(stack != NULL);
    free(stack);
    stack = NULL;
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

