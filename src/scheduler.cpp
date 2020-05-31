#include "scheduler.h"

const int INTHZ = 10;

extern __thread SignalHandler *signalHandler;
extern __thread std::unordered_map<int, Coroutine*> *corMap;

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

void Scheduler::signalProcess(){
    int signal = current->getSignal();
    if(signal != 0){
        for(int signo=1; signo<32; signo++){
            if(signal & (1 << signo))
                assert(signalHandler[signo] != NULL);
                signalHandler[signo](signo);
        }
        current->setSignal();
    }    
}

void Scheduler::wakeup(){
    signalProcess();
    restore(current->getContext());
}

void Scheduler::timerInterrupt(){
    if(!runQue.empty()){
        current = delFromRunQue();
        if(current->getcid()!=0 || (corMap->size() == 1))
            wakeup();
        else{
            addToRunQue(current);
            current = NULL;
        }
    }
}

int Scheduler::schedule(){
    while(true){
        if((current = next()) != NULL) break;
        firedEventSize =  epoll_wait(epollFd, events, maxEventSize, INTHZ);
        if(firedEventSize == 0)
            timerInterrupt();
    }
    wakeup();
}

Scheduler::~Scheduler(){
    close(epollFd);
    free(events);
}

__thread Scheduler *scheduler = NULL;

extern void signalHandlerInit();

void envInitialize(){
    if(scheduler == NULL){
        scheduler = new Scheduler();
    }
    if(signalHandler == NULL){
        signalHandler = new SignalHandler[32];
        signalHandlerInit();
    }
    if(corMap == NULL){
        corMap = new std::unordered_map<int, Coroutine*>;
    }
    printf("env initialize\n");
}

void envDestroy(){
    if(scheduler != NULL){
        delete scheduler;
    }
    if(signalHandler != NULL){
        delete signalHandler;
        signalHandler = NULL;
    }
    if(corMap != NULL){
        delete corMap;
        corMap = NULL;
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
