/*
 * Copyright (c) 2020, xie wenwu <870585356@qq.com>
 * 
 * All rights reserved.
 */

#ifndef __LOAD__
#define __LOAD__

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/syscall.h>

struct CpuStat{
	char cpu[8];
	unsigned long user_time;
	unsigned long nice_time;
	unsigned long sys_time;
	unsigned long idle_time;
	unsigned long iowait_time;
	unsigned long irq_time;
	unsigned long sirq_time;
};

struct TaskStat{
	int pid;                       
	char cmd[16];                
	char state;               
	int ppid;                       
	int group;                      
	int session;                    
	int tty_nr;                     
	int tty_pgrp;                   
	unsigned long flags;         
	unsigned long min_flt;          
	unsigned long cmin_flt;         
	unsigned long maj_flt;          
	unsigned long cmaj_flt;
	unsigned long utime;            
	unsigned long stime;            
	long cutime;                    
	long cstime;                    
	long prio;                  
	long nice;                      
	int num_threads;                
	long it_real_value;             
	unsigned long long start_time;  
	unsigned long vsize;            
	long rss;                       
	unsigned long rlim_cur;         
	unsigned long start_code;       
	unsigned long end_code;         
	unsigned long tart_stack;       
	unsigned long esp;              
	unsigned long eip;              
	unsigned long pending;          
	unsigned long blocked;          
	unsigned long sigign;           
	unsigned long sigcatch;         
	unsigned long wchan;            
	unsigned long nswap;            
	unsigned long cnswap;           
	int exit_signal;                
	int cpu_num;                      
	long rt_prio;               
	long policy;           
};

int getCpuCount();

class Stat{
private:
    int cpuFd, procFd, threadFd;
    
    CpuStat cpuStat;
    
    void openStat();
    void closeStat();
    
    unsigned long getTaskTime(int fd);
    
public:
    Stat(){
        openStat();
    }
    
    inline void getCpuStat();
    
    unsigned long getProcTime(){
        return getTaskTime(procFd);
    }
    
    unsigned long getThreadTime(){
        return getTaskTime(threadFd);
    }
    
    unsigned long getCpuTime(){
        return cpuStat.user_time + cpuStat.nice_time + cpuStat.sys_time + cpuStat.idle_time + cpuStat.iowait_time + cpuStat.irq_time + cpuStat.sirq_time;
    }
    
    unsigned long getIdleTime(){
        return cpuStat.idle_time;
    }

    ~Stat(){
        closeStat();
    }
};

class Load{
private:
    int statCount;
    Stat stat;
    time_t statTime;
    unsigned long threadT1, threadT2, procT1, procT2, cpuT1, cpuT2, idleT1, idleT2;

public:
    int procUsage, procUsage_, threadUsage, threadUsage_, sysUsage, sysUsage_;
    int cpuCount;
    
public:
    Load();
    
    bool cacl();
    
    ~Load(){}
};

#endif
