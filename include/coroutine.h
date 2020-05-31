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

class Coroutine{
private:
    friend void startCoroutine();
    
    int fd;
    int type;
    
    Routine routine;
    void *stack;
    int stackSize;
    void *arg;
    Context context;
	int cid;
    
    int signal;

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
    
    void start();

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
