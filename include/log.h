/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __LOG__
#define __LOG__
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>


extern int gettid();
extern int getcid();

#define INFO "INFO", __FILE__, __LINE__
#define WARN "WARN", __FILE__, __LINE__
#define ERROR "ERROR", __FILE__, __LINE__

static inline void logPrint(const char* lev, const char *file, int line, const char *format, ...){
    char buf[256];
    time_t now=time(NULL);
    struct tm date;
    localtime_r(&now,&date);
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d %s(%d) tid:%d cid:%d %s -> ",
                    date.tm_year+1900, date.tm_mon+1, date.tm_mday,
                    date.tm_hour, date.tm_min, date.tm_sec,
                    basename(file), line, gettid(), getcid(), lev);
    int len = strlen(buf);
    va_list ap;
    va_start(ap, format);
    vsnprintf(buf+len, 256, format, ap);
    va_end(ap);
    printf("%s\n", buf);
}

#define log logPrint

#endif
