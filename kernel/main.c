#include "string.h"
#include "x86.h"
#include "memlayout.h"
#include "memory.h"
#include "pcb.h"
#include "pcb_struct.h"
#include "pmap.h"
#include "cpu.h"
#include "fcb.h"

int printk(const char *fmt, ...);
void init_page();
void init_serial();
void init_ide();
void init_intr();
void init_segment();
void init_idt();
void init_timer();
void init_mm();
void loader();

void keyboard_event(int scan_code);
void init_cond();
/***********************************/
#define MAX_PCB 100
extern struct PCB pcb_table[MAX_PCB];
extern struct PCB *current;
extern PDE* get_kpdir();

void init() {
	init_page();
	asm volatile("addl %0, %%esp" : : "i"(KOFFSET));
	asm volatile("jmp *%0" : : "r"(init_cond));
}
void init_cond()
{
	init_serial();
	init_segment();
	init_idt();
	init_intr();
//	init_ide();
	init_timer();

	init_mm();
	printk("Hello, QEMU world!\n");

	pcb_init();
	file_init();
	inode_init();
//get pcb for init process
	int pcb_index = get_pcb();
	int pid = get_pid();
	pcb_new(pid, 0, pcb_index);
	current = &pcb_table[pcb_index];//!!!

	PDE *kpdir = (PDE*)PADDR(get_kpdir());
	CR3 cr3;
	cr3.val = 0;
	cr3.page_directory_base = ((uint32_t)kpdir) >> 12;
	pcb_cr3write(pcb_index, cr3.val);

	printk("idle's pcb_index = %d,	idle's pid = %d\n", pcb_index, pid);

	loader();
	printk("loader is over\n");
	enable_interrupt();
	while(1)
		wait_intr();
	while(1);
	//panic("should not reach here");
}
