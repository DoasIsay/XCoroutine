#include "scheduler.h"

const int INTHZ = 10;

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

int Scheduler::wait(int fd, int type){
	int ret = 0;

	if(current != NULL){
		ret = save(current->getContext());
		if(ret) 
			return 1;
	}else 
		goto Sched;
	
	if(fd != -1){
		if(current->getFd() == -2) 
			current->setFd(fd);
		epoll_event event;
		event.events = type;
		event.data.ptr = (void*)current;
		ret = epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event);
		if(ret == -1){
			printf("epoll add fd:%d error:%s  \n", fd, strerror(errno));
			return -1;
		}
	}else{
		addToRunQue(current);
	}
	current = NULL;
	
	Sched:
	schedule();
}

void Scheduler::timerInterrupt(){
	if(!runQue.empty()){
		current = removeFromRunQue();
		assert(current != NULL);
		restore(current->getContext());
	}
}

int Scheduler::schedule(){
	int ret = 0;

	while((current = next()) == NULL){
		firedEventSize =  epoll_wait(epollFd, events, maxEventSize, INTHZ);
		if(firedEventSize == 0) 
			timerInterrupt();
	}
	ret = epoll_ctl(epollFd, EPOLL_CTL_DEL, current->getFd(), NULL);
	if(ret == -1){
		printf("epoll del fd:%d error:%s  \n",current->getFd(),strerror(errno));
		return -1;
	}
	
	assert(current != NULL);
	restore(current->getContext());
}

Scheduler::~Scheduler(){
	free(events);
}

__thread Scheduler *scheduler = NULL;

void envInitialize(){
	if(scheduler == NULL){
		scheduler = new Scheduler();
	}
	printf("initialize scheduler\n");
}

void envDestroy(){
	if(scheduler != NULL){
		delete scheduler;
	}
	printf("destory scheduler\n");
}

void startCoroutine(){
	switch(current->routine(current->arg)){
		case -1:printf("%d coroutine fd:%d exit fail\n", syscall(__NR_gettid), current->fd);break;
		case  0:
		case  1:printf("%d coroutine fd:%d exit sucess\n", syscall(__NR_gettid), current->fd);break;
	}

	if(current!=NULL){
		delete current;
		current = NULL;
	}
	
	printf("%d schedule to next\n" ,syscall(__NR_gettid));
	schedule();
}

int waitOnRead(int fd){
	scheduler->wait(fd, Scheduler::READ);
}

int waitOnWrite(int fd){
	scheduler->wait(fd, Scheduler::WRITE);
}

void schedule(){
	scheduler->schedule();
}

void addToRunQue(Coroutine *co){
	scheduler->addToRunQue(co);
}