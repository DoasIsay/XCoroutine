#include "coroutine.h"

const int STACKSIZE = 4096;

extern void startCoroutine();

__thread Coroutine *current = NULL;

Coroutine::Coroutine(int (*routine)(void *), void *arg){
	setjmp(&context);
	this->routine = routine;
	this->arg = arg;
	fd = *(int*)arg;
	context.ip = (long)startCoroutine;
	stack = NULL;
	stackSize = STACKSIZE;
	coroutines++;
}

int Coroutine::setStackSize(int size){
	this->stackSize = size;
}

void Coroutine::start(){
	stack = malloc(stackSize);
	context.sp = (long)((char*)stack + stackSize);
	longjmp(&context);
}

Coroutine::~Coroutine(){
	if(stack!=NULL)
	free(stack);
	coroutines--;
}

int Coroutine::coroutines = 0;

void createCoroutine(int (*routine)(void *),void *arg){
	current = new Coroutine(routine,arg);
	current->start();
}
