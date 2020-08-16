/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __ATOMIC__
#define __ATOMIC__

class Atomic{
private:
    volatile long count;

public:
    Atomic(int value=0):count(value){}
    
    long inc(int val = 1){
        return __sync_add_and_fetch(&count, val);
    }

    long dec(int val = 1){
        return __sync_sub_and_fetch(&count, val);
    }

    long get(){
        return count;
    }

    void set(long count){
        this->count = count;
    }
    
    bool cas(long oldVal, long newVal){
        return __sync_bool_compare_and_swap(&count, oldVal, newVal);
    }

};

static inline bool cas(long *ptr, long oldVal, long newVal){
    return __sync_bool_compare_and_swap(ptr, oldVal, newVal);
}

#define membar do{ __sync_synchronize(); }while(0)

#endif
