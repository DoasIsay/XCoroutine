/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __CORMAP__
#define __CORMAP__

#include <stdlib.h>
#include <memory.h>

class Map{
private:
    int caps;
    int sizes;
    void **arr;
    
    int expand(){
        int newCaps = caps*2;
        void **p= (void**)realloc(arr, newCaps);
            
        if(!p)
            return -1;
        else{
            arr = p;
            int tmpSize = sizeof(void*) * caps;
            memset((char*)arr + tmpSize, 0 , tmpSize);
            caps = newCaps;
            return 0;
        }
    }
        
public: 
    Map(int caps){
        int tmpSize = sizeof(void*) * caps;
        arr = (void**)malloc(tmpSize);
        memset(arr, 0 , tmpSize);
        this->caps = caps;
    }
    
    int set(int key, void *value){
        int ret = 0;
        if(key < caps){
            arr[key] = value;
        }else{
            ret = expand();
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
        if(key < caps){
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
    
    static __thread CorMap *instance;
    
    CorMap(){
        map1 = new Map(1024);
        map2 = new Map(1024);
    }

public:
    static const int STARTCID = 100000000;
    
    static CorMap *Instance(){
        if(!instance)
            instance = new CorMap();
        return instance;
    }

    int set(int key, Coroutine *value){
        if(key < STARTCID){
            return map1->set(key, value);
        }else{
            return map2->set(key - STARTCID, value);
        }
    }

    Coroutine *get(int key){
        if(key < STARTCID){
            return (Coroutine*)map1->get(key);
        }else{
            return (Coroutine*)map2->get(key - STARTCID);
        }
    }

    int del(int key){
        if(key < STARTCID){
            return map1->del(key);
        }else{
            return map2->del(key - STARTCID);
        }
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

