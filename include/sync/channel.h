#include "cond.h"
#include "atomic.h"
#include "../raii.h"

template<class Type>
class Channel{
public:
    class Node{
        public:
            Node():next(NULL){}
            
            Type item;
            volatile Node *next;
    };
    
private:
    //cacheline伪共享暂不解决
    //volatile Node *p01,*p02,*p03,*p04,*p05,*p06,*p07;
    volatile Node *head;
    //volatile Node *p11,*p12,*p13,*p14,*p15,*p16,*p17;
    volatile Node *tail;
    
    int caps;
    Atomic sizes;
    
    Mutex pushMutex, popMutex;
    Cond  condNotFull, condNotEmpty;
    
    Node *freelist;
    
public:
    Channel(int cap){
        this->caps = cap;
        
        //使用静态链表管理空闲的node
        freelist = new Node[cap + 1];
        for(int i = 0; i < cap; i++){
           freelist[i].next = freelist + i + 1;
        }
        freelist[cap].next = NULL;
        
        tail = head = new Node;
    }
    
    Node *alloc(){
        Node *node = NULL;
        
        //cas从无锁freelist获取一个node
        //第一次进入read memory barrier，同步其它cpu核心对freelist->next的最新修改
        membar;
        do{
            while((node = (Node*)freelist->next) == NULL){
                log(INFO, "channel full size %d wait", size());
                if(condNotFull.wait(pushMutex) < 0)
                    return NULL;
            }
        }while(!cas((long*)&freelist->next, (long)node, (long)node->next));
        
        return node;
    }
    
    void free(Node *node){
        //cas归还一个node到无锁freelist
        //第一次进入read memory barrier，同步其它cpu核心对freelist->next的最新修改
        membar;
        do{
            node->next = freelist->next;
        }while(!cas((long*)&freelist->next, (long)node->next, (long)node));
        
        condNotFull.signal();
    }
    
    int push(const Type &item){
        Raii raii(&pushMutex);
        
        Node *node = alloc();
        if(node == NULL) return -1;
        node->next = NULL;
        node->item = item;
        
        tail->next = node;
        tail = node;
        
        sizes.inc();
        
        condNotEmpty.signal();
        return 0;
    }
    
    Channel<Type> &operator << (const Type &item){
        push(item);
        return *this;
    }
    
    int pop(Type &item){
        Node *node = NULL;
        Raii raii(&popMutex);
        
        while((node = (Node*)head->next) == NULL){
            log(INFO, "channel empty size %d wait", size());
            if(condNotEmpty.wait(popMutex) < 0)
                return -1;
        }
        free((Node*)head);
        item = node->item;
        head = node;
        
        sizes.dec();
        
        return 0;
    }

    Channel<Type> &operator >> (Type &item){
        pop(item);
        return *this;
    }
    
    int size(){
        return sizes.get();
    }
    
    int cap(){
        return caps;
    }

    bool empty(){
        return size() == 0;
    }
};
