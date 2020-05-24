#ifndef __SOCKET__
#define __SOCKET__
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "scheduler.h"

int setNoBlock(int fd, int block=1);
int readn(int fd,char *buf, int len);
int writen(int fd,char *buf, int len);

#endif
