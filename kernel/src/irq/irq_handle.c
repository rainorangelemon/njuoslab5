#include "x86.h"
#include "stdio.h"
#include "assert.h"
#include "pcb.h"
#include "pcb_struct.h"

void do_syscall(struct TrapFrame *tf);

struct PCB *current;

uint32_t time_tick;
void schedule();  //kernel_timer_event

void Keyboard_event();

void
irq_handle(struct TrapFrame *tf) {
//	printk("irq %d\n", tf->irq);
	if(tf->irq == 0x80)
	{
			current->tf = tf;
			do_syscall(tf);
	}
	else if(tf->irq < 1000) {
		switch(tf->irq){ 
			case 0: printk("Divide Error!\n"); break;
			case 1: printk("Debug Exceptions!\n"); break;
			case 3: printk("Breakpoint Error!\n"); break;
			case 4: printk("Overflow Error!\n"); break;
			case 5: printk("Bounds Check Error!\n"); break;
			case 6: printk("Invalid Opcode Error!\n"); break;
			case 7: printk("Coprocessor Not Available!\n"); break;
			case 8: printk("Double Fault!\n"); break;
			case 9: printk("Coprocessor Segment Overrun!\n"); break;
			case 10: printk("Invalid TSS!\n"); break;
			case 11: printk("Segment Not Present!\n"); break;
			case 12: printk("Stack Exception!\n"); break;
			case 13: printk("General Protection Exception!\n"); break;
			case 14: {
						 printk("Fault in Page!\n");
						 if((tf->error_code & 0x1) == 0) 
							printk("The page does not present!\n");
						 else{
							 if(tf->error_code & 0x2) printk("The access causing the fault was a write.\n");
							 else printk("The access causing the fault was a read.\n");
							 if(tf->error_code & 0x4) printk("The processor was executing in user mode.\n");
							 else printk("The processor was executing in supervisor mode.\n");
						 }
						 break;
			}
			default: printk("Unexisted exception!\n"); break;
		}
		printk("Unhandled exception! irq==%d,error_code=0x%x\n", tf->irq, tf->error_code);
		while(1);
	}
	else if (tf->irq == 1000) {
		
		time_tick++;
		wakeup();		//all the process with blocked get sleep-1
		if(current->time_count == 10)// current has run for enough period
		{
			current->tf = tf;
			process_ready(current->pcb_index);
			schedule();
		}
		else
		{
			current->tf = tf;
			current->time_count++;
		}
	} else if (tf->irq == 1001) {
		current->tf = tf;
		Keyboard_event();
	} else if(tf->irq == 1014) {
		current->tf = tf;
	}
	else {
		printk("wrong\n");
	}
}

