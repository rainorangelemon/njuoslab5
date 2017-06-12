#include "mmu.h"
#include "irq.h"
#include "video.h"
#include "x86.h"
#include "pcb.h"
#include "pcb_struct.h"
#include "semaphore.h"
#include "fcb.h"

int getpid();
void serial_printc(char ch);
void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *v, int c, size_t n);
int handle_keys();

#define KEYCODE 0
#define VMEMORY 1
#define WRITE   2
#define TIME    3
#define GETPID  4
#define FORK 	5
#define SLEEP 	6
#define EXIT 	7
#define THREAD	8
#define SEM_INIT	9
#define SEM_WAIT	10
#define SEM_POST	11
#define SEM_DESTROY	12
#define SEM_TRYWAIT	13
#define FILE_OPEN      14
#define FILE_READ      15
#define FILE_WRITE     16
#define FILE_LSEEK     17
#define FILE_CLOSE     18

int fs_write(int fd, void *buf, int len){
	int ret=-1;
	if(fd==1){
		ret=len;
		int i;
		for(i=0;i<ret;++i){
			serial_printc(*(char*)(buf+i));
		}
	}
	return ret;
}

extern uint32_t time_tick;
extern struct PCB* current;
void do_syscall(struct TrapFrame *tf) {
	switch(tf->eax) {
		case KEYCODE: //get keyboard code
			tf->eax=handle_keys();
			break;
		case WRITE: //mostly used for printf
			tf->eax = fs_write(tf->ebx,(void*)tf->ecx,tf->edx);
			break;
		case VMEMORY: //draw the screen
			memcpy(VMEM_ADDR, (void *)tf->ebx, SCR_SIZE);
			break;
		case TIME: //get the time_tick
			tf->eax=time_tick;
			break;
		case SLEEP: //sleep
			process_sleep(tf->ebx);
			break;
		case FORK:
			process_fork();
			break;
		case EXIT:
			process_exit();
			break;
		case GETPID:
			tf->eax=getpid();
			break;
		case THREAD:
			create_thread((uint32_t *)tf->ebx);
			break;
		case SEM_INIT:
			sem_init((sem_t *)tf->ebx, (int)tf->ecx, (bool)tf->edx);
			break;
		case SEM_WAIT:
			sem_wait((sem_t *)tf->ebx);
			break;
		case SEM_POST:
			sem_post((sem_t *)tf->ebx);
			break;
		case SEM_DESTROY:
			sem_destroy((sem_t *)tf->ebx);
			break;
		case SEM_TRYWAIT:
			tf->eax=sem_trywait((sem_t *)tf->ebx);
			break;
		case FILE_OPEN:
			tf->eax=open((char *)tf->ebx, tf->ecx);
			break;
		case FILE_READ:
			tf->eax=read(tf->ebx, (char *)tf->ecx, tf->edx);
			break;
		case FILE_WRITE:
			tf->eax=write(tf->ebx, (char *)tf->ecx, tf->edx);
			break;
		case FILE_LSEEK:
			tf->eax=lseek(tf->ebx, tf->ecx, tf->edx);
			break;
		case FILE_CLOSE:
			tf->eax=close(tf->ebx);
			break;

		default: printk("Unhandled system call: id = %d", tf->eax);
	}
}
