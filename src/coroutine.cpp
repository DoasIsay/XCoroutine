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
const static int MAXCOS = 65536*128;

static int allocCid(){
    int cid = 0;
    CorMap *corMap = CorMap::Instance();
    for(cid = CorMap::STARTCID ; corMap->get(cid) != NULL; ++cid);
    return cid; 
}

Coroutine::Coroutine(int (*routine)(void *), void *arg){
    next = NULL;
    stack = NULL;
    this->arg = arg;
    this->routine = routine;
    stackSize = STACKSIZE;
    signal = 0;
    id.cid = allocCid();
}

Coroutine::Coroutine(int cid){
    this->id.cid = cid;
    signal = 0;
    next = NULL;
    //CorMap::Instance()->set(cid, this);
}

int Coroutine::setStackSize(int size){
    this->stackSize = size;
}

void Coroutine::start(){
    if(id.cid < 0)
        return;
    save(&context);
    context.pc = (long)startCoroutine;
    
    stack = malloc(stackSize);
    assert(stack != NULL);
    
    context.sp = (long)((char*)stack + stackSize);
    CorMap::Instance()->set(id.cid, this);
    addToRunQue(this);
}

Coroutine::~Coroutine(){
    if(stack != NULL)
        free(stack);
    CorMap::Instance()->del(id.cid);
}

void createCoroutine(int (*routine)(void *),void *arg){
    Coroutine *co= new Coroutine(routine,arg);
    co->start(); 
}

int getcid(){
    if(current == NULL)
        return 0;
    return current->getcid();
}
int gettid(){
    return syscall(__NR_gettid);
}
