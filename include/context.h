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
    void restore(Context *cxt, long funPtr);
}

#endif
