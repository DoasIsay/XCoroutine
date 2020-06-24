/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __QUEUE__
#define __QUEUE__
//a first in first out queue, only used when item is point

#include "locker.h"

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
        item->next = NULL;

        locker.lock();
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
        T item = NULL;
        locker.lock();
        if(head != NULL){
            item = head;
            head = head->next;
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
