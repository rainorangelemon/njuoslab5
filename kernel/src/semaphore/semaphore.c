#include "types.h"
#include "semaphore_struct.h"
#include "pcb_struct.h"
#include "x86.h"

#define New 	0
#define Ready	1
#define Running	2
#define Blocked	3

#define MAX_PCB 100

extern struct PCB *current;
extern struct PCB pcb_table[MAX_PCB];
int printk(const char *fmt, ...);
void schedule();
void release_sem(int);

int sem_count = 0;	//the index of the semaphore

//because these operations are atom operations, so I should use closing the interruptions firstly.

void sem_init(sem_t *sem, int value, bool mutex)
{
	disable_interrupt();
	printk("value %d, mutex %d\n", value, mutex);
	if(mutex)
	{
		sem->value = 1;
		sem->mutex = true;
		sem->sem_index = sem_count;		// the index of current semaphore
	printk("sem_index %d\n", sem->sem_index);
		sem->block_proc_num = 0;		//initialize the blocked process to 0
	}
	else
	{
		sem->value = value;
		sem->mutex = false;
		sem->sem_index = sem_count;
	printk("sem_index %d\n", sem->sem_index);
		sem->block_proc_num = 0;
	}
	sem_count ++;
	enable_interrupt();
}

void sem_wait(sem_t *sem)
{
	disable_interrupt();
	if(!sem->mutex)
	{
		sem->value = sem->value - 1;
		if(sem->value < 0)
		{
			current->status = Blocked;
			current->block_sem_index = sem->sem_index;	//the index of semaphore which makes the current process blocked
			sem->block_proc_num ++;		//+1 to the current semaphore's blocked process
			enable_interrupt();
			schedule();
		}
	}
	else	//mutex
	{
		if(sem->value == 1)
			sem->value = 0;
		else
		{
			current->status = Blocked;
			current->block_sem_index = sem->sem_index;
			sem->block_proc_num ++;
			enable_interrupt();
			schedule();
		}
	}
	enable_interrupt();
}

void sem_post(sem_t *sem)
{
	disable_interrupt();
	if(!sem->mutex)
	{
		sem->value = sem->value + 1;
		if(sem->value <= 0)
		{
			sem->block_proc_num --;
			release_sem(sem->sem_index);	
		}
	}
	else	
	{
		if(sem->block_proc_num == 0)//no thread/process is blocked by this sem
			sem->value = 1;
		else
		{
			sem->block_proc_num --;
			release_sem(sem->sem_index);
		}
	}
	enable_interrupt();
}

void sem_destroy(sem_t *sem)
{
	disable_interrupt();
	release_sem(sem->sem_index);
	enable_interrupt();
}

int sem_trywait(sem_t *sem)
{
	disable_interrupt();
	if(!sem->mutex)
	{
		sem->value = sem->value - 1;
		if(sem->value < 0)
		{
			enable_interrupt();
			return 1;
		}
	}
	else	//mutex
	{
		if(sem->value == 1)
			sem->value = 0;
		else
		{
			enable_interrupt();
			return 1;
		}
	}
	enable_interrupt();
	return 0;
}

