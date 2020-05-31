#include "coroutine.h"

extern void addToRunQue(Coroutine *co);
extern void startCoroutine();

__thread Coroutine *current = NULL;
__thread std::unordered_map<int, Coroutine*> *corMap = NULL;

const static int STACKSIZE = 4096;
const static int MAXCOS = 65536*128;

static int allocCid(){
    static __thread int nextCid = 0;
    do{
        nextCid++;
        if(corMap->size() >= MAXCOS){
            printf("exceed max coroutines %d\n", MAXCOS);
            return -1;
        }
        if(nextCid >= MAXCOS)
            nextCid = 1;
    }while(corMap->find(nextCid) != corMap->end());
    return nextCid;
}

Coroutine::Coroutine(int (*routine)(void *), void *arg){
    fd = -1;
    stack = NULL;
    this->arg = arg;
    this->routine = routine;
    stackSize = STACKSIZE;
    signal = 0;
}

Coroutine::Coroutine(int cid){
    this->cid = cid;
    signal = 0;
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

Coroutine::~Coroutine(){
    if(stack != NULL)
        free(stack);
    corMap->erase(cid);
}

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
