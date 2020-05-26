#include "coroutine.h"

const int STACKSIZE = 4096;

extern void addToRunQue(Coroutine *co);
extern void startCoroutine();

__thread Coroutine *current = NULL;

Coroutine::Coroutine(int (*routine)(void *), void *arg){
    this->routine = routine;
    this->arg = arg;
    fd = -2;
    stack = NULL;
    stackSize = STACKSIZE;
    coroutines++;
    save(&context);
    context.pc = (long)startCoroutine;
}

int Coroutine::setStackSize(int size){
    this->stackSize = size;
}

void Coroutine::start(){
    stack = malloc(stackSize);
    context.sp = (long)((char*)stack + stackSize);
    addToRunQue(this);
}

Coroutine::~Coroutine(){
    if(stack != NULL)
        free(stack);
    coroutines--;
}

int Coroutine::coroutines = 0;

void createCoroutine(int (*routine)(void *),void *arg){
    Coroutine *co= new Coroutine(routine,arg);
    co->start(); 
}
