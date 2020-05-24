#ifndef __SETJMP__
#define __SETJMP__
typedef struct JmpBuf{
	long rbx,rcx,rdx,sp,rbp,rsi,r12,r13,r14,r15,ip;
}JmpBuf;

extern "C"{
	int setjmp(JmpBuf *cxt);
	void longjmp(JmpBuf *cxt);
}
#endif

