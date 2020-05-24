#ifndef __COROUTINE__
#define __COROUTINE__
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "setjmp.h"

typedef int(*Routine)(void *);
void createCoroutine(int (*routine)(void *),void *arg);

class Coroutine{
private:
	friend void startCoroutine();
	static int coroutines;
	int type;
	int fd;
	Routine routine;
	void *stack;
	int stackSize;
	void *arg;
	JmpBuf context;

public:
	
	Coroutine(int (*routine)(void *), void *arg);
	
	int setStackSize(int size);

	int setJmp(JmpBuf &jmpBuf){
		context = jmpBuf;
	}
	JmpBuf* getJmp(){
		return &context;
	}
	
	void setType(int type){
		this->type = type;
	}
	
	int getFd(){
		return this->fd;
	}

	static int getCoroutines(){
		return coroutines;
	}
	
	void start();

	~Coroutine();
};

void createCoroutine(int (*routine)(void *),void *arg);
#endif
