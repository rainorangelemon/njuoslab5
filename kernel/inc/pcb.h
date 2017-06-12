#ifndef __PCB_H__
#define __PCB_H__

#include "mmu.h"
/*#define KSTACK_SIZE 4096
struct PCB { 
	struct TrapFrame *tf;
	uint8_t kstack[KSTACK_SIZE];

	int pid;                //Process id
	int ppid;               //Parent process id
	int state;              //Process state
	int time_count;         //the running times of Process 
	int sleep_time;         //Sleep count down
	uint32_t cr3;           //PDT base address
	int pcb_index;
};*/

int get_pid();
void pcb_init();
int get_pcb();
void pcb_new(int pid, int ppid, int pcb_index);
void process_run(int pcb_index);
void process_ready(int pcb_index);
void wakeup();
void process_sleep(int time);
void process_fork();
void pcb_cr3write(int, uint32_t);
void copy(int);
void process_exit();
void create_thread(uint32_t *func_addr);
void thread_copy(int);
#endif
