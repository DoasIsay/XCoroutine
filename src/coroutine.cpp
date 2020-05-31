#include "coroutine.h"

extern void addToRunQue(Coroutine *co);
extern void startCoroutine();

__thread Coroutine *current = NULL;
__thread std::unordered_map<int, Coroutine*> *corMap = NULL;

Coroutine::Coroutine(int (*routine)(void *), void *arg){
    this->routine = routine;
    this->arg = arg;
    fd = -1;
    stack = NULL;
    stackSize = STACKSIZE;
    coroutines++;
}
Coroutine::Coroutine(int cid){
      this->cid = cid;
      corMap->insert(std::make_pair(cid, this));
 }

int Coroutine::setStackSize(int size){
    this->stackSize = size;
}

void Coroutine::start(){
    cid = allocCid();
    if(cid < 0)
        return;
    save(&context);
    context.pc = (long)startCoroutine;
    stack = malloc(stackSize);
    assert(stack != NULL);
    context.sp = (long)((char*)stack + stackSize);
    corMap->insert(std::make_pair(cid, this));
    addToRunQue(this);
}

int Coroutine::allocCid(){
    do{
        nextCid++;
        if(coroutines >= MAXCOS){
            printf("exceed max coroutines %d\n", MAXCOS);
            return -1;
        }
        if(nextCid >= MAXCOS)
            nextCid = 1;
    }while(corMap->find(nextCid) != corMap->end());
    return nextCid;
}

Coroutine::~Coroutine(){
    if(stack != NULL)
        free(stack);
    corMap->erase(cid);
}

__thread int Coroutine::coroutines = 0;
__thread int Coroutine::nextCid = 0;

SignalHandler* Coroutine::signalHandler = new SignalHandler[32];

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
