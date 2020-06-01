/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#include "coroutine.h"

typedef void (*SignalHandler)(int);
extern SignalHandler signalHandler[];

int ckill(int cid, char signo);
int csignal(SignalHandler handler);

void defaultHandler(int signo);
