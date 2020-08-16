#include <iostream>
#include <signal.h>
#include <pthread.h>
#include "coroutine.h"
#include "sync.h"
#include "log.h"

using namespace std;

bool isExit = false;

Atomic atomic;
Channel<int> channel(10);

int producer(void *){
    while(!isExit){
        int item = atomic.inc();
        log(INFO, "producer %d", item);
        //channel.push(item);
        channel<<item<<item;
    }
}

int consumer(void *){
    while(!isExit){
        int item = -1;
        //channel.pop(item);
        channel>>item;
        log(INFO, "consumer %d", item);
    }
}

void *Producer(void *){
    createCoroutine(producer, NULL);
    createCoroutine(producer, NULL);
    createCoroutine(producer, NULL);
    createCoroutine(producer, NULL);
    createCoroutine(producer, NULL);
    createCoroutine(producer, NULL);
    createCoroutine(producer, NULL);
    createCoroutine(producer, NULL);
    createCoroutine(producer, NULL);
    yield;
}

void *Consumer(void *){
    createCoroutine(consumer, NULL);
    createCoroutine(consumer, NULL);
    createCoroutine(consumer, NULL);
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
    //csignal(SIGTERM, ignore);

    atomic.set(0);
    pthread_t t0,t1,t2;
    pthread_create(&t0, NULL, Consumer, NULL);
    pthread_create(&t1, NULL, Consumer, NULL);
    pthread_create(&t2, NULL, Producer, NULL);
    
    pthread_join(t0, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    log(INFO, "exit sucess");
}
