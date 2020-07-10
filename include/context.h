/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __CONTEXT__
#define __CONTEXT__
#include <string.h>
#include <stdlib.h>
#include "log.h"

typedef struct Context{

#ifdef __i386__
    long ebx,ecx,edx,sp,ebp,esi,edi,pc;
#elif __x86_64__
    long rbx,rcx,rdx,sp,rbp,rsi,r12,r13,r14,r15,pc;
#endif

}Context;

extern "C"{
    int save(Context *cxt);
    void restore(Context *cxt, long funPtr);
}

static inline char *allocStack(int stackSize){
    char *stack = (char*)malloc(stackSize);
    assert(stack != NULL);
    
    #ifdef STACK_CHECK
    memcpy(stack + stackSize - 8, "12345678", 8);
    #endif
    
    return stack;
}

static inline long getStackPoint(){
    long sp = 0;
    #ifdef __i386__
    asm("movl %%esp,%0;":"=m"(sp):);
    #elif __x86_64__
    asm("movq %%rsp,%0;":"=m"(sp):);
    #endif
    
    return sp;
}

#ifdef STACK_CHECK

#define STACK_OVERFLOW_CHECK(_LOW_STACK_POINT_, _STACK_SIZE_)\
        do{\
            char *_HIGH_STACK_POINT_ = _LOW_STACK_POINT_ + _STACK_SIZE_;\
            long _CUR_STACK_POINT_ = getStackPoint();\
            if((unsigned long)_CUR_STACK_POINT_ < (unsigned long)_LOW_STACK_POINT_){\
                log(ERROR, "stack overflow low stack point: %x, cur stack point: %x",\
                    _LOW_STACK_POINT_, _CUR_STACK_POINT_);\
                exit(-1);\
            }\
            if(memcmp((void*)(_HIGH_STACK_POINT_ - 8), "12345678", 8)){\
                log(ERROR, "stack corrupted low stack point: %x, high stack point: %x %s",\
                    _LOW_STACK_POINT_, _HIGH_STACK_POINT_ -8, (_HIGH_STACK_POINT_ - 8));\
                exit(-1);\
            }\
        }while(0)
#else

#define STACK_OVERFLOW_CHECK(_LOW_STACK_POINT_, _STACK_SIZE_)

#endif

#endif

