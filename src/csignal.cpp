#include "csignal.h"

extern __thread std::unordered_map<int, Coroutine*> *corMap;

int ckill(int cid, char signo){
    std::unordered_map<int, Coroutine*>::iterator findIt = corMap->find(cid);
    if(findIt != corMap->end()){
        findIt->second->sendSignal(signo);
        return 0;
    }else
        return -1;
}

int csignal(int signo, SignalHandler handler){
    Coroutine::signalHandler[signo] = handler;
}


