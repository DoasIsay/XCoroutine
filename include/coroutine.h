#ifndef __COROUTINE__
#define __COROUTINE__
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "context.h"
#include <set>
#include <assert.h>

typedef int(*Routine)(void *);
void createCoroutine(int (*routine)(void *),void *arg);

class Coroutine{
private:
    const static int STACKSIZE = 4096;
    const static int MAXCOS = 65536*128;
    friend void startCoroutine();
    static __thread  int coroutines;
    int fd;
    Routine routine;
    void *stack;
    int stackSize;
    void *arg;
    Context context;
	int cid;
	static __thread int nextCid;

public:
    
    Coroutine(int (*routine)(void *), void *arg);
	
    Coroutine(int cid);
	
    int setStackSize(int size);

    int setContext(Context &cxt){
        context = cxt;
    }

    Context* getContext(){
        return &context;
    }

    void setFd(int fd){
        this->fd = fd;
    }
    int getFd(){
        return fd;
    }

    static int getCoroutines(){
        return coroutines;
    }
    
    void start();

    int allocCid();

    int getcid(){
        return cid;
    }
	
    ~Coroutine();
};

void createCoroutine(int (*routine)(void *),void *arg);
int getcid();
int gettid();
#endif
