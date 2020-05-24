#ifndef __SCHEDULER__
#define __SCHEDULER__
#include <stdio.h>
#include <sys/epoll.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <assert.h>
#include "context.h"
#include "coroutine.h"
#include "queue.h"

extern __thread Coroutine *current;

void envInitialize();
void envDestroy();

void startCoroutine();

class Scheduler{
private:
	int epollFd;
	int nextEventIdx;
	int firedEventSize;
	int maxEventSize;
	epoll_event *events;
	Queue<Coroutine*> runQue;
	
public:
	static const int READ=EPOLLIN, WRITE=EPOLLOUT;
	Scheduler(int max);

	Coroutine* next();

	int wait(int fd, int type);
	
	void addToRunQue(Coroutine* co){
		runQue.push(co);
	}
	Coroutine* removeFromRunQue(){
		return runQue.pop();
	}
	
	void timerInterrupt();
	
	int schedule();
	
	~Scheduler();

};

extern __thread Scheduler *scheduler;

int waitOnRead(int fd);
int waitOnWrite(int fd);

void schedule();

#define yield do{ scheduler->wait(-1, -1); }while(0)
#endif
