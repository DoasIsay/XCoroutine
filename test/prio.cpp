#include <iostream>
#include <signal.h>
#include "coroutine.h"
#include "log.h"

using namespace std;

int test1(void *){
    cout<<"test1"<<endl;
    sleep(1);
    cout<<"test1"<<endl;
}

int test2(void *){
    cout<<"test2"<<endl;
    sleep(1);
    cout<<"test2"<<endl;
}

int test3(void *){
    cout<<"test3"<<endl;
    sleep(1);
    cout<<"test3"<<endl;
}

int main(){
    Coroutine *co1 = new Coroutine(test1, NULL);
    Coroutine *co2 = new Coroutine(test2, NULL);
    Coroutine *co3 = new Coroutine(test3, NULL);
    
    co1->setPrio(8);
    co2->setPrio(5);
    co3->setPrio(1);

    co1->start();
    co2->start();
    co3->start();
    
    yield;
    
    log(INFO, "exit sucess");
}
