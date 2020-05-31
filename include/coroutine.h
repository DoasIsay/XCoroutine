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
#include <unordered_map>

typedef int(*Routine)(void *);
void createCoroutine(int (*routine)(void *),void *arg);

typedef void (*SignalHandler)(int);

class Coroutine{
private:
    const static int STACKSIZE = 4096;
    const static int MAXCOS = 65536*128;
    friend void startCoroutine();
    static __thread  int coroutines;
    int fd;
    int type;
    Routine routine;
    void *stack;
    int stackSize;
    void *arg;
    Context context;
	int cid;
	static __thread int nextCid;
    int signal;

public:
    static SignalHandler *signalHandler;

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

    int setType(int type){
        this->type = type;
    }
    int getType(){
        return type;
    }
    
    static int getCoroutines(){
        return coroutines;
    }
    
    void start();

    int allocCid();

    int getcid(){
        return cid;
    }

    int sendSignal(int signo){
        signal|=signo;
    }

    int getSignal(){
        return signal;
    }

    void setSignal(){
        signal = 0;
    }
    
    ~Coroutine();
};

void createCoroutine(int (*routine)(void *),void *arg);
int getcid();
int gettid();
#endif
