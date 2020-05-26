#ifndef __QUEUE__
#define __QUEUE__
//a first in first out queue

template<typename T>
class Node{
public:
    Node *next;
    Node *pre;
    T item;
    Node(){
        next = pre = NULL;
    }
};

template<typename T>
class Queue{
private:
    int size;
    Node<T> *head, *tail;
    
public:
    Queue(){
        size = 0;
        head = tail = NULL;
    }
    
    void push(T item){
        Node<T> *tmp = new Node<T>();
        tmp->item = item;
        if(head == NULL){
            head = tail = tmp;
        }else{
            head->pre = tmp;
            tmp->next = head;
            head = tmp;
        }
        size++;
    }

    //the item is a point,use this fun is better, but it may maked two times obj copy
    T pop(){
        if(tail != NULL){
            T tmpItem = tail->item;
            Node<T> *tmpNode = tail;
            tail = tail->pre;
            delete tmpNode;
            if(tail != NULL)
                tail->next = NULL;
            else
                head = NULL;
            size--;
            return tmpItem;
        }else{
            throw "Queue is empty";
        }
    }

    int pop(T &item){
        if(tail != NULL){
            Node<T> *tmp = tail;
            item = tmp->item;
            tail = tail->pre;
            delete tmp;
            if(tail != NULL)
                tail->next = NULL;
            size--;
            return 0;
        }else{
            return -1;
        }
    }

    int empty(){
        return size == 0;
    }
};
#endif
