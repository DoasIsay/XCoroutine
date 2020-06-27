/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */
#include "coroutine.h"

typedef void (*SignalHandler)(int);
extern SignalHandler signalHandler[];

int ckill(int cid, int signo);
int ckill(Coroutine *co, int signo);
int csignal(int signo, SignalHandler handler);

void sigdefHandler(int signo);
