#include <iostream>
#include <signal.h>
#include <pthread.h>
#include "coroutine.h"
#include "log.h"
#include <list>
#include "sync.h"

using namespace std;

bool isExit = false;

int i =0;
SpinLocker locker;
Sem sem(10);
list<int> que;

int producer(void *){
    while(!isExit){
        sem.wait();

        locker.lock();
        int tmp = i++;
        que.push_back(i);
        log(INFO, "producer %d", tmp);
        locker.unlock();
    }
}

int consumer(void *){
    while(!isExit){
        locker.lock();
        if(!que.empty()){
            int tmp = que.front();
            log(INFO, "consumer %d", tmp);
            que.pop_front();
        }
        locker.unlock();
        
        sem.signal();
    }
}


void *Consumer(void *){
    createCoroutine(consumer, NULL);
    createCoroutine(consumer, NULL);
    createCoroutine(consumer, NULL);
    yield;
}

void *Producer(void *){
    createCoroutine(producer, NULL);
    createCoroutine(producer, NULL);
    createCoroutine(producer, NULL);
    yield;
}

void quit(int signo)
{
    isExit = true;
    stopCoroutines();
}

void ignore(int signo){
    cout<<"ignore signo "<<signo<<endl;
}

int main(){
    signal(SIGINT, quit);
    signal(SIGTERM, quit);
    csignal(SIGTERM, ignore);
    
    pthread_t t0,t1,t2;
    pthread_create(&t0, NULL, Consumer, NULL);
    pthread_create(&t1, NULL, Consumer, NULL);
    pthread_create(&t2, NULL, Producer, NULL);
    
    pthread_join(t0, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    cout<<"que size "<<que.size()<<endl;
    
    log(INFO, "exit sucess");
}

