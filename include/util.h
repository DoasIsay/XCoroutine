#ifndef __UTIL__
#define __UTIL__
#include <sys/time.h>
static inline long milliseconds(){
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return tv.tv_usec/1000;
}

static inline long seconds(){
    return time(NULL);
}

#endif