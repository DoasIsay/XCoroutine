/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __CONTEXT__
#define __CONTEXT__
typedef struct Context{

#ifdef __i386__
    long ebx,ecx,edx,sp,ebp,esi,edi,pc;
#elif __x86_64__
    long rbx,rcx,rdx,sp,rbp,rsi,r12,r13,r14,r15,pc;
#endif

}Context;

extern "C"{
    int save(Context *cxt);
    void restore(Context *cxt);
}

static long getStackPoint(){
    long sp = 0;
    #ifdef __i386__
        asm("movl %%esp,%0;":"=m"(sp):);
    #elif __x86_64__
        asm("movq %%rsp,%0;":"=m"(sp):);
    #endif

    return sp;
}

#define STACK_OVERFLOW_CHECK(_END_STACK_POINT_)\
        do{\
            long _CUR_STACK_POINT_ = getStackPoint();\
            if((unsigned long)_CUR_STACK_POINT_ < (unsigned long)_END_STACK_POINT_){\
                log(ERROR, "stackoverflow stack point: %x, cur stack point: %x", _END_STACK_POINT_, _CUR_STACK_POINT_);\
                exit(-1);\
            }\
        }while(0)

#endif

