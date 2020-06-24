/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __CORMAP__
#define __CORMAP__

#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "locker.h"

class Map{
private:
    volatile int caps;
    volatile int sizes;
    void** volatile arr;
    
    int expand(){
        int newCaps = caps * 2;
        int newBytes = sizeof(void*) * newCaps;
        void **p= (void**)realloc(arr, newBytes);
            
        if(!p)
            return -1;
        else{
            arr = p;
            int oldBytes= sizeof(void*) * caps;
            memset((char*)arr + oldBytes, 0 , oldBytes);
            caps = newCaps;
            return 0;
        }
    }
        
public: 
    Map(int caps){
        int bytes = sizeof(void*) * caps;
        arr = (void**)malloc(bytes);
        memset(arr, 0 , bytes);
        this->caps = caps;
    }
    
    int set(int key, void *value){
        int ret = 0;
        if(key < caps){
            arr[key] = value;
        }else{
            ret = expand();
            if(ret != 0){
                return ret;
            }
            arr[key] = value;
        }
        sizes++;
        return ret;
    }
    
    void *get(int key){
        if(key < caps)
            return arr[key];
        else
            return NULL;
    }

    int del(int key){
        if(key < caps && arr[key] != NULL){
            sizes--;
            arr[key] = NULL;
            return 0;
        }
        return -1;
    }

    int size(){
        return sizes;
    }

    int cap(){
        return caps;
    }
        
    ~Map(){
        if(arr)
            free(arr);
    }
};

class Coroutine;

class CorMap{
private:
    Map *map1;
    Map *map2;
    
    SpinLocker locker;

    static volatile CorMap *instance;
    CorMap(){
        map1 = new Map(1024);
        map2 = new Map(1024);
        
    }

    CorMap(const CorMap&) = delete;
    CorMap &operator=(const CorMap&) = delete;
    
public:
    static const int STARTCID = 100000000;
    
    static CorMap *Instance(){
        static SpinLocker locker;
        if(instance != NULL)
            return (CorMap*)instance;
        locker.lock();
        if(!instance)
            instance = new CorMap();
        locker.unlock();
        return (CorMap*)instance;
    }

    int set(int key, Coroutine *value){
        locker.lock();
        int ret;
        if(key < STARTCID){
            ret = map1->set(key, value);
        }else{
            ret = map2->set(key - STARTCID, value);
        }
        locker.unlock();
        return ret;
    }

    Coroutine *get(int key){
        Coroutine *co = NULL;
        locker.lock();
        if(key < STARTCID){
            co = (Coroutine*)map1->get(key);
        }else{
            co = (Coroutine*)map2->get(key - STARTCID);
        }
        locker.unlock();
        return co;
    }

    int del(int key){
        int ret = 0;
        locker.lock();
        if(key < STARTCID){
            ret = map1->del(key);
        }else{
            ret = map2->del(key - STARTCID);
        }
        locker.unlock();
        return ret;
    }

    int size(){
        return map1->size() + map2->size();
    }

    int empty(){
        return size() == 0;
    }
    
    ~CorMap(){
        if(map1)
            delete map1;
        if(map2)
            delete map2;
    }
};

#endif

