#ifndef __SOCKET__
#define __SOCKET__
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

namespace net{
    int setNoBlock(int fd, int block=1);
    int readn(int fd,char *buf, int len);
    int writen(int fd,char *buf, int len);
    int accept(int fd);
}
#endif
