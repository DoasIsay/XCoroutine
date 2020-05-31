#include "scheduler.h"

const int INTHZ = 10;

extern int getcid();
extern int gettid();

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

    assert(current != NULL);
    if(save(current->getContext()))
        return 1;
    
    if(fd > 0){
        if(current->getFd() < 0){
            current->setFd(fd);
            current->setType(type);
            eventAdd(fd, type);
        }else if(current->getType() != type){
            current->setType(type);
            eventMod(fd, type);
        }else if(type == WRITE)
            eventMod(fd, type);
            
    }else{
        addToRunQue(current);
    }

    current = NULL;
    schedule();
}

void Scheduler::timerInterrupt(){
    if(!runQue.empty()){
        current = delFromRunQue();
        if(current->getcid()!=0 || (cidSet->size() == 1))
            restore(current->getContext());
        else
            addToRunQue(current);
    }
}

int Scheduler::schedule(){
    while((current = next()) == NULL){
        firedEventSize =  epoll_wait(epollFd, events, maxEventSize, INTHZ);
        if(firedEventSize == 0)
            timerInterrupt();
    }
    restore(current->getContext());
}

Scheduler::~Scheduler(){
    free(events);
    close(epollFd);
}

__thread Scheduler *scheduler = NULL;

void envInitialize(){
    if(scheduler == NULL){
        scheduler = new Scheduler();
    }
    if(cidSet == NULL){
        cidSet = new std::set<int>;
    }
    printf("env initialize\n");
}

void envDestroy(){
    if(scheduler != NULL){
        delete scheduler;
    }
    if(cidSet != NULL){
        delete cidSet;
    }
    printf("env destory\n");
}

void startCoroutine(){
    switch(current->routine(current->arg)){
        case -1:printf("%d:%d coroutine fd:%d exit fail\n", gettid(), getcid(), current->fd);break;
        case  0:
        case  1:printf("%d:%d coroutine fd:%d exit sucess\n", gettid(), getcid(), current->fd);break;
    }

    if(current!=NULL){
        scheduler->eventDel(current->getFd(), current->getType());
        delete current;
        current = NULL;
    }
    
    printf("%d schedule to next\n", gettid());
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
