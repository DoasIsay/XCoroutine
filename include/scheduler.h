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
    
    int eventCtl(int fd, int type, int mode){
        epoll_event event;
        event.events = type|EPOLLET;
        if(type == WRITE)   event.events |= EPOLLONESHOT;
        event.data.ptr = (void*)current;
        return epoll_ctl(epollFd, mode, fd, &event);
    }
    
    int eventAdd(int fd, int type){
        return eventCtl(fd, type, EPOLL_CTL_ADD);
    }
    
    int eventDel(int fd, int type){
        return eventCtl(fd, type, EPOLL_CTL_DEL);
    }

    int eventMod(int fd, int type){
        return eventCtl(fd, type, EPOLL_CTL_MOD);
    }

    int wait(int fd, int type);
    
    void addToRunQue(Coroutine* co){
        runQue.push(co);
    }
    Coroutine* delFromRunQue(){
        return runQue.pop();
    }
    
    void timerInterrupt();
    
    int schedule();

    void wakeup();
    
    ~Scheduler();

};

extern __thread Scheduler *scheduler;

void envInitialize();
void envDestroy();

void schedule();
void startCoroutine();

int waitOnRead(int fd);
int waitOnWrite(int fd);

#define yield\
	do{\
		if(current == NULL){\
			current = new Coroutine(0);\
		}\
		scheduler->wait(-1, -1);\
	}while(0)
#endif
