#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include "scheduler.h"
#include "socket.h"
#include <signal.h>

bool isExit = false;

int readWriteRoutine(void *arg){
    int fd;
    int recbytes;
    int sin_size;
    char buffer[32]={0};   
    struct sockaddr_in s_add,c_add;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == fd)
    {
        printf("create socket fail error:%s\n",strerror(errno));
        return -1;
    }
    
    bzero(&s_add,sizeof(struct sockaddr_in));
    s_add.sin_family=AF_INET;
    s_add.sin_addr.s_addr= inet_addr("127.0.0.1");
    s_add.sin_port=htons(5566);
    
    if( connect(fd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr))<0)
    {
        printf("connect fail error:%s \n",strerror(errno));
        return -1;
    }
    net::setNoBlock(fd);
    
    char buf[]="I am coming,,,,,,,";

    while(!isExit){
        net::writen(fd,buf,sizeof(buf));
        log(INFO, "send %s\n",buf);
        net::readn(fd,buf,sizeof(buf));
        log(INFO, "recv %s\n",buf);
    }
    close(fd);
}

void quit(int signo)
{
	isExit = true;
}


int main(int argvs, char *argv[])
{
    signal(SIGTERM,quit);

    for(int i=0; i<1000; i++)       
    createCoroutine(readWriteRoutine, NULL);
    yield;
    return 0;
}
