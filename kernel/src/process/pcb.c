#include "types.h"
#include "mmu.h"
#include "pcb.h"
#include "pcb_struct.h"
#include "string.h"
#include "cpupa.h"
#define MAX_PCB 100

#define New 	0
#define Ready	1
#define Running	2
#define Blocked	3

#define Sleeptime 5

#define NR_FILE 32

int printk(const char *fmt, ...);
extern struct PCB *current;
void set_tss_esp0(uint32_t);
PDE* get_updir();
void release();
void *memset(void *v, int c, size_t n);

struct PCB pcb_table[MAX_PCB];
int pid_count = 0;			//where is the highest pid

int get_pid()
{
	pid_count++;
	return pid_count;
}

void pcb_init()		//initialize
{
	int i;
	for(i = 0; i < MAX_PCB; i ++)
	{
		pcb_table[i].status = -1;
		pcb_table[i].pid = -1;
		pcb_table[i].block_sem_index = -1;
		memset(pcb_table[i].fcb,-1,sizeof(pcb_table[i].fcb));
	}
}

int get_pcb()	//get a pcb for process
{
	int pcb_index;
	for(int i = 0; i < MAX_PCB; i ++)
	{
		if(pcb_table[i].status == -1)
		{
			pcb_index = i;
			break;
		}
	}
	return pcb_index;		//give pcb_index
}

void pcb_new(int pid, int ppid, int pcb_index)
{
	pcb_table[pcb_index].pid = pid;
	pcb_table[pcb_index].ppid = ppid;
	pcb_table[pcb_index].status = New;
	pcb_table[pcb_index].time_count = 0;
	pcb_table[pcb_index].sleep_time = Sleeptime;
	pcb_table[pcb_index].pcb_index = pcb_index;
}

void pcb_cr3write(int pcb_index, uint32_t val)
{
	pcb_table[pcb_index].cr3 = val;
}

void process_ready(int pcb_index)
{
		pcb_table[pcb_index].status = Ready;
		pcb_table[pcb_index].time_count = 0;
}

void process_run(int pcb_index)
{
		pcb_table[pcb_index].status = Running;
		pcb_table[pcb_index].time_count += 1;
}

void schedule()
{
	int i;
	for(i = current->pcb_index + 1; i % MAX_PCB != current->pcb_index; i=(i+1)%MAX_PCB)
	{
		if(pcb_table[i].status == Ready)
		{
			current = &pcb_table[i];
			current->status = Running;
			set_tss_esp0((uint32_t)(pcb_table[i].kstack + 4096));
			write_cr3(pcb_table[i].cr3);
			return;
		}
	}
	current = &pcb_table[0];
}

void wakeup()
{
	for(int i = 0; i < MAX_PCB; i ++)
	{
		if(pcb_table[i].status == Blocked)
		{
			pcb_table[i].sleep_time --;
			if(pcb_table[i].sleep_time == 0)
				pcb_table[i].status = Ready;
		}
	}
}

void process_sleep(int time)
{
	current->status = Blocked;
	current->sleep_time = time;
	schedule();
	return;
}

void process_exit()
{
	current->status = -1;
	current->pid = -1;
	release();
	printk("proc %d exit\n", current->pid);
	schedule();
}

int getpid()	//just get pid
{
	return current->pid;
}

void process_fork()
{
	current->tf->eax = current->pid;
	current->status = Ready;
			
	int pcb_index = get_pcb();	//new pcb
	int pid = -1; // the son process's pid is -1;
	
	pcb_new(pid, getpid(), pcb_index);
	process_ready(pcb_index);
	copy(pcb_index);
	
	//copy trap frame
	memcpy(pcb_table[pcb_index].kstack, current->kstack, KSTACK_SIZE);

	//copy fcb
	memcpy(pcb_table[pcb_index].fcb, current->fcb, sizeof(int)*NR_FILE);
	
	//modify kernel stack
	struct TrapFrame *tf = (struct TrapFrame *)(pcb_table[pcb_index].kstack + 4096 - sizeof(struct TrapFrame));
	pcb_table[pcb_index].tf = tf;

	tf->eax = pid;
	
	schedule();
}

void create_thread(uint32_t *func_addr)  //in this kernel, I view thread as special process with some same property with each other
{
	int pcb_index = get_pcb();
	int pid = -2;		//-2 means it's a thread

	pcb_new(pid, getpid(), pcb_index);
	process_ready(pcb_index);
	thread_copy(pcb_index);
	//set trap frame
	struct TrapFrame *tf = (struct TrapFrame *)(pcb_table[pcb_index].kstack + 4096 - sizeof(struct TrapFrame));
	tf->ss = current->tf->ss;
	tf->esp = 0xC0000000 - 4;
	tf->eflags = 0x202;
	tf->cs = current->tf->cs;
	tf->eip = (uint32_t)func_addr;
	tf->ds = current->tf->ds;
	tf->es = current->tf->es;
	pcb_table[pcb_index].tf = tf;
	
}

void release_sem(int sem_index)
{
	for(int i = 0; i < MAX_PCB; i ++)
	{
		if(pcb_table[i].block_sem_index == sem_index)
		{
			pcb_table[i].block_sem_index = -1;
			pcb_table[i].status = Ready;
			pcb_table[i].time_count = 0;
			break;
		}
	}
}
