/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __QUEUE__
#define __QUEUE__
//a first in first out queue, only used when item is point

template<class T>
class Queue{
private:
    int sizes;
    T head, tail;
    
public:
    Queue(){
        sizes = 0;
        head = tail = NULL;
    }
    
    void push(T item){
        assert(item != NULL);
        item->next = NULL;
        
        if(tail == NULL){
            head = tail = item;
        }else{
            tail->next = item;
            tail = item;
        }
        sizes++;
    }

    T pop(){
        T item = NULL;
        if(head != NULL){
            item = head;
            head = head->next;
            sizes--;
            if(!head) 
                tail = NULL;
        }
        return item;
    }

	int size(){
		return sizes;
	}
	
    int empty(){
        return size() == 0;
    }
};
#endif
