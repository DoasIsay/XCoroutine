#include <iostream>
#include <signal.h>
#include <pthread.h>
#include "coroutine.h"
#include "log.h"
#include <list>
#include "cond.h"

using namespace std;

bool isExit = false;

int i =0;
Mutex mutex;
Cond condNotFull, condNotEmpty;
list<int> que;

int producer(void *){
    while(!isExit){
        mutex.lock();
        
        while(que.size() == 10 && !isExit){
            log(INFO, "que full %d wait", que.size());
            condNotFull.wait(mutex);
            log(INFO, "wakeup");
        }
        
        int tmp = i++;
        que.push_back(i);
        log(INFO, "producer %d", tmp);

        mutex.unlock();
        condNotEmpty.signal();
    }
    condNotEmpty.broadcast();
}

int consumer(void *){
    while(!isExit){
        mutex.lock();
        
        while(que.empty() && !isExit){
            log(INFO, "que empty %d wait", que.size());
            condNotEmpty.wait(mutex);
            log(INFO, "wakeup");
        }

        if(!que.empty()){
            int tmp = que.front();
            log(INFO, "consumer %d", tmp);
            que.pop_front();
        }
        
        mutex.unlock();
        condNotFull.signal();
    }
    condNotFull.broadcast();
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
