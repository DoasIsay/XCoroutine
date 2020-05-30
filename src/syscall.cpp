#include <dlfcn.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

extern int waitOnRead(int fd);
extern int waitOnWrite(int fd);

typedef ssize_t (*SysRead)(int fd, void *buf, size_t count);
typedef ssize_t (*SysWrite)(int fd, const void *buf, size_t count);
typedef int (*SysAccept)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

SysRead sysRead	= (SysRead)dlsym(RTLD_NEXT, "read");
SysWrite sysWrite = (SysWrite)dlsym(RTLD_NEXT, "write");
SysAccept sysAccept= (SysAccept)dlsym(RTLD_NEXT, "accept");

ssize_t read(int fd, void *buf, size_t count){
    int ret = sysRead(fd, buf, count);
    if(ret < 0){
        if(errno == EAGAIN){
            waitOnRead(fd);
        }
    }
    return ret;
}

ssize_t write(int fd, const void *buf, size_t count){
    int ret = sysWrite(fd, buf, count);
    if(ret < 0){
        if(errno == EAGAIN){
            waitOnWrite(fd);
        }
    }
    return ret;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    int ret = sysAccept(sockfd, addr, addrlen);
    if(ret < 0){
        if(errno == EAGAIN){
            waitOnRead(sockfd);
        }
    }
    return ret;
}
