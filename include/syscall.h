
/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __SYSCALL__
#define __SYSCALL__
typedef int (*SysClose)(int fd);
typedef int (*SysDup)(int oldfd);
typedef int (*SysListen)(int sockfd, int backlog);
typedef unsigned int (*SysSleep)(unsigned int seconds);
typedef ssize_t (*SysRead)(int fd, void *buf, size_t count);
typedef int (*SysSocket)(int domain, int type, int protocol);
typedef ssize_t (*SysWrite)(int fd, const void *buf, size_t count);
typedef int (*SysAccept)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
typedef int (*SysConnect)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

extern SysDup sysDup        ;
extern SysRead sysRead	    ;
extern SysClose sysClose    ;
extern SysSleep sysSleep    ;
extern SysWrite sysWrite    ;
extern SysListen sysListen  ;
extern SysSocket sysSocket  ;
extern SysAccept sysAccept  ;
extern SysConnect sysConnect;

#endif