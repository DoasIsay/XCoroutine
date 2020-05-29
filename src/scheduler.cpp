#include "scheduler.h"

bool isExit = false;
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
        int ret = 0;
        epoll_event event;
        event.events = type|EPOLLONESHOT;
        event.data.ptr = (void*)current;
        if(current->getFd() < 0){
            current->setFd(fd);
            ret = epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event);
        }else{
            ret = epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);
        }
        if(ret < 0){
            printf("epoll add fd:%d error:%s  \n", fd, strerror(errno));
            return -1;
        }
    }else{
        addToRunQue(current);
    }

    current = NULL;
    schedule();
}

void Scheduler::timerInterrupt(){
    if(!runQue.empty()){
        current = removeFromRunQue();
        if(current->getcid()!=0 || (cidSet->size() == 1 && isExit))
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