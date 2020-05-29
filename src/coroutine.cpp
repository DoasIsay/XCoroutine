#include "coroutine.h"

extern bool isExit;

extern void addToRunQue(Coroutine *co);
extern void startCoroutine();

__thread Coroutine *current = NULL;
__thread std::set<int> *cidSet = NULL;

Coroutine::Coroutine(int (*routine)(void *), void *arg){
    this->routine = routine;
    this->arg = arg;
    fd = -1;
    stack = NULL;
    stackSize = STACKSIZE;
    coroutines++;
    cid = allocCid();
}
Coroutine::Coroutine(int cid){
      this->cid = cid;
      cidSet->insert(cid);
 }

int Coroutine::setStackSize(int size){
    this->stackSize = size;
}

void Coroutine::start(){
    save(&context);
    context.pc = (long)startCoroutine;
    stack = malloc(stackSize);
    assert(stack != NULL);
    context.sp = (long)((char*)stack + stackSize);
    addToRunQue(this);
}

int Coroutine::allocCid(){
    do{
        nextCid++;
        if(nextCid >= MAXCOS)
            nextCid = 1;
    }while(cidSet->find(nextCid) != cidSet->end());
    cidSet->insert(nextCid);
    return nextCid;
}

Coroutine::~Coroutine(){
    if(stack != NULL)
        free(stack);
    coroutines--;
    cidSet->erase(cid);
}

__thread int Coroutine::coroutines = 0;
__thread int Coroutine::nextCid = 0;

void createCoroutine(int (*routine)(void *),void *arg){
    Coroutine *co= new Coroutine(routine,arg);
    co->start(); 
}

int getcid(){
    return current->getcid();
}
int gettid(){
    return syscall(__NR_gettid);
}
