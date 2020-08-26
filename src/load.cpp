/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#include "load.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int getCpuCount(){
    FILE *fp = fopen("/proc/cpuinfo", "r");
    assert(fp != NULL);

    char *pos = NULL;
    char buf[1024] = {0};
    char process[32] = {0};
    
    while(fgets(buf, sizeof(buf), fp) != NULL){    
        if((pos = strstr(buf, "processor")) == NULL) continue;
        strcpy(process, buf);
    }
    
    pos = strstr(process, ":");
    
    return atoi(pos + 1) + 1;   
}

void Stat::openStat(){
    pid_t pid = getpid();
    pid_t tid = syscall(__NR_gettid);
    
    char path[32];
    sprintf(path, "/proc/%d/stat", pid);
    procFd = open(path, O_RDONLY);
    
    sprintf(path, "/proc/%d/task/%d/stat", pid, tid);
    threadFd = open(path, O_RDONLY);
    
    cpuFd  =  open("/proc/stat", O_RDONLY);
}

void Stat::closeStat(){
    close(cpuFd);
    close(procFd);
    close(threadFd);
}

void Stat::getCpuStat(){
    char buff[256] = {0};
    
    lseek(cpuFd, 0, SEEK_SET);
    read(cpuFd, buff, 256);
    
    sscanf(buff,"%s %lu %lu %lu %lu %lu %lu %lu",cpuStat.cpu, &cpuStat.user_time, &cpuStat.nice_time,
                                                &cpuStat.sys_time, &cpuStat.idle_time, &cpuStat.iowait_time,
                                                &cpuStat.irq_time, &cpuStat.sirq_time);
}

unsigned long Stat::getTaskTime(int fd){
    char buff[512] = {0};
    
    lseek(fd, 0, SEEK_SET);
    read(fd, buff, 512);
    
    TaskStat stat;
    sscanf(buff,"%d %s %c %d %d %d %d %d %lu %lu \                                                       
            %lu %lu %lu %lu %lu %ld %ld %ld %ld %d %ld %llu %lu %ld %lu %lu %lu %lu %lu \
            %lu %lu %lu %lu %lu %lu %lu %lu %d %d %lu %lu\n",
            &stat.pid, stat.cmd, &stat.state, &stat.ppid, &stat.group, &stat.session,
            &stat.tty_nr, &stat.tty_pgrp, &stat.flags, &stat.min_flt, &stat.cmin_flt, &stat.maj_flt,
            &stat.cmaj_flt, &stat.utime, &stat.stime, &stat.cutime, &stat.cstime, &stat.prio, &stat.nice,
            &stat.num_threads, &stat.it_real_value, &stat.start_time, &stat.vsize, &stat.rss, &stat.rlim_cur,
            &stat.start_code, &stat.end_code, &stat.tart_stack, &stat.esp, &stat.eip, &stat.pending, &stat.blocked,
            &stat.sigign, &stat.sigcatch, &stat.wchan, &stat.nswap, &stat.cnswap, &stat.exit_signal, &stat.cpu_num, &stat.rt_prio, &stat.policy);
    
    return stat.utime + stat.stime + stat.cutime + stat.cstime;
}

Load::Load(){
    sysUsage = procUsage = threadUsage = 0;
    sysUsage_ = procUsage_ = threadUsage_ = 0;
    
    cpuCount = getCpuCount();
        
    cpuT1 = stat.getCpuTime();
    procT1 = stat.getProcTime();
    idleT1 = stat.getIdleTime();
    threadT1 = stat.getThreadTime();
        
    statTime = time(NULL);
    statCount = 0;
}

bool Load::cacl(){
    time_t now = time(NULL);
    if(now - statTime < 1) return false;
    statTime = now;
    statCount++;
    
    stat.getCpuStat();
    cpuT2 = stat.getCpuTime();
    procT2 = stat.getProcTime();
    idleT2 = stat.getIdleTime();
    threadT2 = stat.getThreadTime();
    
    unsigned long cpuTime = cpuT2 - cpuT1;
    
    sysUsage_ += (100 - (idleT2 - idleT1) * 100/ cpuTime);
    procUsage_ += ((procT2 - procT1) * 100 / cpuTime * cpuCount);
    threadUsage_ += ((threadT2 - threadT1) * 100 / cpuTime * cpuCount);
    
    cpuT1 = cpuT2;
    procT1 = procT2;
    idleT1 = idleT2;
    threadT1 = threadT2;
    
    if(statCount == 10){
        sysUsage = sysUsage_ / statCount;
        procUsage = procUsage_ / statCount;
        threadUsage = threadUsage_ / statCount;

        sysUsage_ = procUsage_ = threadUsage_ = 0;
        statCount = 0;
    }
    return true;
}

