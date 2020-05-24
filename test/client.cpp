#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h> 
#include <time.h>


int main(int argvs, char *argv[])
{
	int fd;
	int recbytes;
	int sin_size;
	char buffer[1024]={0};   
	struct sockaddr_in s_add,c_add;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == fd)
	{
	    printf("create socket fail \n");
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
	char buf[]="I am coming,,,,,,,";
	printf("%d\n",sizeof(buf));
	while(true){
		sleep(10);
		printf("send %s\n",buf);
		write(fd,buf,sizeof(buf));
		read(fd,buf,sizeof(buf));
		printf("recv %s\n",buf);
	}
	close(fd);
	return 0;
}
