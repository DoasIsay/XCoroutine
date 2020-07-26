/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __QUEUE__
#define __QUEUE__
//a first in first out queue, only used when item is point

#include "locker.h"

template<class T>
bool isLinked(T item){
    return item->next != item;
}

template<class T, class LockerType = Locker>
class Queue{
private:
    LockerType locker;
    volatile int sizes;
    volatile T head, tail;
    
public:
    Queue(){
        sizes = 0;
        head = tail = NULL;
    }
    
    void push(T item){
        assert(item != NULL);
        if(isLinked(item))
            return;
        

        locker.lock();
        item->next = NULL;
        if(tail == NULL){
            head = tail = item;
        }else{
            tail->next = item;
            tail = item;
        }
        sizes++;
        locker.unlock();
    }

    T pop(){
        locker.lock();
        T item = NULL;
        if(head != NULL){
            item = head;
            head = head->next;
            item->next = item;
            sizes--;
            if(!head) 
                tail = NULL;
        }
        locker.unlock();
        return item;
    }

    int size(){
        return sizes;
    }
	
    bool empty(){
        return size() == 0;
    }
};

#endif
