#include "coroutine.h"

typedef void (*SignalHandler)(int);

int ckill(int cid, char signo);

int csignal(SignalHandler handler);
