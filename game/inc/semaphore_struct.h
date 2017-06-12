#ifndef MYSEMAPHORE_H
#define MYSEMAPHORE_H

#include "types.h"
typedef struct sem_t{
	int value;
	bool mutex;
	int sem_index;			//信号量的编号
	int block_proc_num;		//阻塞了多少个进程
}sem_t;

#endif
