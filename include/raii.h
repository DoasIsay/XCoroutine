#ifndef __RAII__
#define __RAII__

class Raii{
private:
    Locker *locker;
    
public:
    Raii(Locker *locker){
        this->locker = locker;
        locker->lock();
    }

    ~Raii(){
        locker->unlock();
        locker = NULL;
    }
};

#endif
