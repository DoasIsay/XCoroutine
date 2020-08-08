#include <iostream>
#include <signal.h>
#include <pthread.h>
#include "coroutine.h"
#include "mutex.h"
#include "log.h"

using namespace std;

bool isExit = false;

Mutex mutex;

int i =0;

int test(void *){
    while(!isExit){
        mutex.lock();
        if(i == 10000000){
            break;
        }
        i++;
        mutex.unlock();
    }
    mutex.unlock();
}

void *fun(void *){
    createCoroutine(test, NULL);
    createCoroutine(test, NULL);
    createCoroutine(test, NULL);
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
    pthread_create(&t0, NULL, fun, NULL);
    pthread_create(&t1, NULL, fun, NULL);
    pthread_create(&t2, NULL, fun, NULL);
    
    pthread_join(t0, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    cout<<"result "<<i<<endl;
    
    log(INFO, "exit sucess");
}
