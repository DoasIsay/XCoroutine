#include"scheduler.h"

Scheduler::Scheduler(int max =1024){
	nextEventIdx = 0;
	firedEventSize = 0;
	maxEventSize = max;
	epollFd = epoll_create(max);
	events = (epoll_event*)malloc(max*sizeof(epoll_event));
}

Coroutine* Scheduler::next(){
	for(;nextEventIdx<firedEventSize;){
		return (Coroutine*)(events[nextEventIdx++].data.ptr);
	}
	nextEventIdx=firedEventSize=0;
	return NULL;
}

int Scheduler::save(int type, int timeout){
	int ret = 0;
	current->setType(type);
	if(timeout!=0)
		ret = setjmp(current->getJmp());
	if(ret){
		return 1;
	}
	epoll_event event;
	event.events = type;
	event.data.ptr = (void*)current;
	ret = epoll_ctl(epollFd, EPOLL_CTL_ADD, current->getFd(), &event);
	if(ret == -1){
		printf("epoll add fd:%d error:%s  \n",current->getFd(),strerror(errno));
		return -1;
	}
	if(timeout!=0)
		restore(timeout);
	else
		return 0;
}

int Scheduler::restore(int timeOut){
	while((firedEventSize==0)||(current = next())==NULL){
		firedEventSize =  epoll_wait(epollFd,events,maxEventSize,timeOut);
	}
	int ret = epoll_ctl(epollFd, EPOLL_CTL_DEL, current->getFd(), NULL);
	if(ret == -1){
		printf("epoll del fd:%d error:%s  \n",current->getFd(),strerror(errno));
		return -1;
	}
	longjmp(current->getJmp());
}

Scheduler::~Scheduler(){		
	free(events);
}

__thread Scheduler *scheduler = NULL;

int schedule(int type,int timeout){
	return scheduler->schedule(type,timeout);
}

void envInitialize(){
	if(scheduler == NULL){
		scheduler = new Scheduler();
	}
}

void envDestroy(){
	if(scheduler != NULL){
		delete scheduler;
	}
}

void startCoroutine(){
	switch(current->routine(current->arg)){
		case -1:printf("%d coroutine fd:%d exit fail\n" ,getpid(), current->fd);break;
		case  0:
		case  1:printf("%d coroutine fd:%d exit sucess\n" ,getpid(), current->fd);break;
	}

	if(current!=NULL){
		delete current;
		current = NULL;
	}
	
	printf("%d schedule to next\n" ,getpid());
	schedule(Scheduler::NEXT);
}
