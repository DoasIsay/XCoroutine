#ifndef __SCHEDULER__
#define __SCHEDULER__
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "setjmp.h"
#include "coroutine.h"

extern __thread Coroutine *current;

void envInitialize();
void envDestroy();

void startCoroutine();

int schedule(int type,int timeout=1000);

class Scheduler{
private:
	int epollFd;
	int nextEventIdx;
	int firedEventSize;
	int maxEventSize;
	epoll_event *events;
	
public:
	static const int READ=EPOLLIN, WRITE=EPOLLOUT, NEXT=101;
	Scheduler(int max);

	Coroutine* next();

	int save(int type, int timeout);
	int restore(int timeOut);

	int schedule(int type, int timeout){
		if(type == NEXT){
			restore(timeout);
		}
		if(save(type,timeout) == 1){
			return 1;
		}
		return 0;
	}
	
	~Scheduler();

};
#endif
