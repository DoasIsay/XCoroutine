#include <pthread.h>
#include "coroutine.h"
#include "sync.h"
#include "log.h"

int ret[10] = {0};
int arr[1000] = {0};
CountDownLatch latch(10);

int map(void *arg){
    int startIdx = *(int*)arg;
    
    for(int i = startIdx; i < startIdx + 100; i++){
        ret[startIdx/100] += arr[i];
        yield;
    }
    
    latch.countDown();
    log(INFO, "map[%d-%d] = %d exit count %d", startIdx, startIdx + 100, ret[startIdx/100], latch.getCount());
}

int reduce(void *){
    latch.wait();
    
    long sum = 0;
    for(int i = 0; i< 10; i++)
    sum += ret[i];
    log(INFO, "reduce %d exit", sum);
}

void *startMapTask(void *arg){
    int startIdx = *(int*)arg;
    int args[5];
    int j=0;
    
    for(int i = startIdx; i < startIdx + 500; i+=100){
        args[j] = i;
        createCoroutine(map, args + j++);
    }
    
    yield;
    log(INFO, "exit sucess");
}

void *startReduceTask(void *){
    createCoroutine(reduce, NULL);
    yield;
    log(INFO, "exit sucess");
}

int main(){
    for(int i = 1; i <= 1000; i++)
    arr[i-1] = i;
    
    pthread_t t0,t1,t2;
    int startIdx0 = 0, startIdx1 = 500;
    
    pthread_create(&t0, NULL, startReduceTask, NULL);
    pthread_create(&t1, NULL, startMapTask, &startIdx0);
    pthread_create(&t2, NULL, startMapTask, &startIdx1);
    
    pthread_join(t0, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    log(INFO, "exit sucess");
}

