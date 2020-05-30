#include "socket.h"

extern bool isExit;

namespace net{

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
    while(!isExit){
        int ret = read(fd, buf+reads, len-reads);
        if(ret < 0){
            if(errno == EINTR || errno==EAGAIN){
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
    while(!isExit){
        int ret = write(fd, buf+writes, len-writes);
        if(ret < 0){
            if(errno == EINTR || errno==EAGAIN){
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
    while(!isExit){
        if((clientFd = accept(serverFd, (struct sockaddr *)NULL, NULL)) <= 0){
            if(errno == EINTR || errno==EAGAIN){
                continue;
            }else{
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
