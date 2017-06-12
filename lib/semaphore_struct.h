#ifndef MYSEMAPHORE_H
#define MYSEMAPHORE_H

#include "include/types.h"
#include "./pcb_struct.h"

typedef struct s_semaphore{
	int value;
	bool mutex;
	int sem_index;			//信号量的编号
	int block_proc_num;		//阻塞了多少个进程
}sem_t;

#endif
