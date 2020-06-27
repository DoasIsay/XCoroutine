#include <iostream>
#include "scheduler.h"
#include "csignal.h"
#include <signal.h>

using namespace std;

Coroutine *co0 = NULL;
Coroutine *co1 = NULL;
Coroutine *co2 = NULL;
Coroutine *co3 = NULL;

int test0(void *arg){
    cout<<getcid()<<" test0 0"<<endl;
    ckill(co1, SIGHUP);
    yield;
    cout<<getcid()<<" test0 0"<<endl;
    return 0;
}

int test1(void *arg){
    cout<<getcid()<<" test1 1"<<endl;
    ckill(co2, SIGHUP);
    yield;
    cout<<getcid()<<" test1 1"<<endl;
    return 0;
}

int test2(void *arg){
    cout<<getcid()<<" test2 2"<<endl;
    ckill(co0, SIGHUP);
    yield;
    cout<<getcid()<<" test2 2"<<endl;
    return 0;
}

int deadLoop(void *arg){
    int i = 0;
    while(1){
        i++;
        sleep(1);
        cout<<getcid()<<" dead loop "<<i<<endl;
    }
}

void sighup(int signo){
    cout<<getcid()<<" recv sig "<<signo<<endl;
}

void sigterm(int signo){
    cout<<getcid()<<" recv sig "<<signo<<", will stop dead loop coroutine"<<endl;
    ckill(co3, signo);
}

int main(){
    
    signal(SIGTERM, sigterm);
    
    csignal(SIGHUP, sighup);
    
    co0 = createCoroutine(test0, NULL);
    co1 = createCoroutine(test1, NULL);
    co2 = createCoroutine(test2, NULL);
    co3 = createCoroutine(deadLoop, NULL);
    
    yield;
    log(INFO, "exit sucess");
}

