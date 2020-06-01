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
/*
void initCxt(Context &cxt){
#ifdef __i386__
		cxt.ebx=cxt.ecx=cxt.edx=cxt.ebp=cxt.esi=cxt.edi=0;
#elif __x86_64__
		cxt.rbx=cxt.rcx=cxt.rdx=cxt.rbp=cxt.rsi=cxt.r12=cxt.r13=cxt.r14=cxt.r15=0;
#endif

}
*/
#endif

