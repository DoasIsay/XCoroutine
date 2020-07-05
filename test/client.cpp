#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "scheduler.h"
#include <signal.h>

bool isExit = false;

int readWriteRoutine(void *arg){
    
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == fd)
    {
        printf("create socket fail error:%s\n",strerror(errno));
        return -1;
    }
    
    struct sockaddr_in addr;
    bzero(&addr,sizeof(struct sockaddr_in));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr= inet_addr("127.0.0.1");
    addr.sin_port=htons(5566);
    
    if(connect(fd,(struct sockaddr *)(&addr), sizeof(struct sockaddr)) < 0)
    {
        printf("connect fail error:%s \n",strerror(errno));
        return -1;
    }
    
    char buf[]="I am coming,,,,,,,";
    
    while(!isExit){
        int ret = write(fd,buf,sizeof(buf));
        if(ret < 0){
             log(ERROR, "fd:%d read error:%s", fd, strerror(errno));
             break;
        }
        log(INFO, "fd:%d send %s\n", fd, buf);
        
        ret = read(fd,buf,sizeof(buf));
        if(ret < 0){
             log(ERROR, "fd:%d write error:%s", fd, strerror(errno));
             break;
        }
    }
    close(fd);
}

void quit(int signo)
{
    isExit = true;
    stopCoroutines();
}

int main(int argvs, char *argv[])
{
    signal(SIGTERM,quit);
    
    for(int i=0; i<10000 && !isExit; i++)       
    createCoroutine(readWriteRoutine, NULL);
    
    yield;
    
    log(INFO, "exit sucess");
}
