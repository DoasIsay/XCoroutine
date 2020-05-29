#include "socket.h"

extern bool isExit;

extern int waitOnRead(int fd);
extern int waitOnWrite(int fd);

namespace NET{
int setNoBlock(int fd, int block) {
    int flags;
    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        printf("fcntl get error %s", strerror(errno));
        return -1;
    }

    if(block)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) == -1) {
        printf("fcntl set error %s", strerror(errno));
        return -1;
    }
    return 0;
}

int readn(int fd,char *buf, int len){
    int reads = 0;
    while(1){
        int ret = read(fd, buf+reads, len-reads);
        if(ret < 0){
            if(errno == EINTR)
                continue;
            else if(isExit)
                return 0;
            else if(errno==EAGAIN){
                waitOnRead(fd);
                continue;
            }else{
                printf("read error:%s\n", strerror(errno));
                return ret;
            }
        }else if(ret == 0){
            return reads;
        }
        reads += ret;
        if( reads==len ){
            return reads;
        }
    }
}

int writen(int fd,char *buf, int len){
    int writes = 0;
    while(1){
        int ret = write(fd, buf+writes, len-writes);
        if(ret < 0){
            if(errno == EINTR)
                continue;
            else if(isExit)
                return 0;
            else if(errno==EAGAIN){
                waitOnWrite(fd);
                continue;
            }else{
                printf("write error:%s\n", strerror(errno));
                return ret;
            }
        }
        writes += ret;
        if(writes == len){
            return writes;
        }
    }
}

int accept(int fd){
    int serverFd = fd;
    int clientFd = 0;
    while(1){
        if((clientFd = accept(serverFd, (struct sockaddr*)NULL, NULL)) <= 0){
            if(errno==EAGAIN){
                waitOnRead(serverFd);
                continue;
            }else if(isExit)
                return 0;
            else{
                printf("accept error: %s\n", strerror(errno));
                return -1;
            }   
        }
        else{
            setNoBlock(clientFd);
            return clientFd;
        }
    }
}
}
