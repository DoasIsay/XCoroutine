#ifndef __COROUTINE__
#define __COROUTINE__
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "context.h"

typedef int(*Routine)(void *);
void createCoroutine(int (*routine)(void *),void *arg);

class Coroutine{
private:
	friend void startCoroutine();
	static int coroutines;
	int fd;
	Routine routine;
	void *stack;
	int stackSize;
	void *arg;
	Context context;

public:
	
	Coroutine(int (*routine)(void *), void *arg);
	
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

	~Coroutine();
};

void createCoroutine(int (*routine)(void *),void *arg);

#endif
