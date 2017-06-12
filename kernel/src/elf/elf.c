#include "elf.h"
#include "types.h"
#include "cpupa.h"
#include "cpu.h"
#include "memory.h"
#include "mmu.h"
#include "pmap.h"
#include "pcb.h"
#include "pcb_struct.h"
#include "x86.h"
#include "fcb.h"

#define STACK_SIZE (4 * (1 << 20))
uint32_t get_ucr3();
#define SECTSIZE 512

#define NR_PDE 1024

//dir for users
extern PDE updir[NR_PDE];

void readseg(unsigned char *, int, int);
void readsect(void *dst, int offset);
uint32_t mm_malloc(uint32_t, int);
void *memset(void *v, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t len);
void page_init(void);

int printk(const char *fmt, ...);
//void create_video_mapping();

void create_video_mapping();

#define MAX_PCB 100
extern struct PCB pcb_table[MAX_PCB];
extern struct PCB *current;

void loader()
{
	int pcb_index = get_pcb();  
	int pid = get_pid();      

	printk("game's pcb_index = %d 	game's pid = %d\n",pcb_index, pid);
	pcb_new(pid, 0, pcb_index);  

/*********************************************************************/
	struct ELFHeader *elf;
	struct ProgramHeader *ph, *eph;
	struct iNode inode;
	uint8_t buffer[4096];
	uint8_t sec_buf[4096];

	readseg((uint8_t *)&bitmap, 256*512, 0);
	readseg((uint8_t *)&direct, 512, sizeof(bitmap));

	elf = (struct ELFHeader*)buffer;

	uint32_t inode_offset=direct.entry[1].inode_offset;
	readseg((uint8_t*)&inode,512,512*inode_offset);
	uint32_t elf_off=inode.data_block_offset[0];
	readseg((uint8_t *)buffer, 4096, elf_off*512);
	
	ph = (struct ProgramHeader*)((uint8_t*)elf + elf->phoff);
	eph = ph + elf->phnum;

	page_init();
	for(; ph < eph; ph++)
	{
		if(ph->type == PT_LOAD) {
			uint32_t va = ph->vaddr;
			int num = 0;				//the bytes that have already been loaded
			for(; va < ph->vaddr + ph->memsz; va +=PGSIZE)
			{
				int off = PGOFF(va);	//offset in page
				va = PTE_ADDR(va);		//first address for page
				uint32_t addr = mm_malloc(va, PGSIZE);//allocating the pysical page concerning the va
				memset(sec_buf, 0, PGSIZE);		//initial the section of disk
				int rest = PGSIZE - off;		//dividing the page size of actual space
				if(ph->filesz - num < rest)	//if the no rest information exists
					rest = ph->filesz - num;
				if(rest != 0)
				//	readseg((void*)(sec_buf + off), rest, 200*512 + ph->p_offset + num);
				readseg((void *)(sec_buf + off), rest, elf_off*512 + ph->off + num);
				memcpy((void*)addr, sec_buf, PGSIZE);
				num += rest;
			}
		}
	}
/**********************************************/
//stack for user program
	for(int i = KOFFSET - STACK_SIZE; i < KOFFSET; i += PGSIZE)
		mm_malloc(i, PGSIZE);

//video map
	create_video_mapping();

/**********************************************/
//let cr3 br the first address of user's pgdir
	PDE *pdir = (PDE*)PADDR(updir);
	CR3 cr3;
	cr3.val = 0;
	cr3.page_directory_base = ((uint32_t)pdir) >> 12;
	pcb_cr3write(pcb_index, cr3.val);

/**********************************************/

//Set TrapFrame
	uint32_t eip = elf->entry;
	uint32_t cs = SELECTOR_USER(SEG_USER_CODE);
	uint32_t ss = SELECTOR_USER(SEG_USER_DATA);
	uint32_t ds = ss;
	uint32_t es = ss;

	struct TrapFrame *tf = (struct TrapFrame*)(pcb_table[pcb_index].kstack + 4096 - sizeof(struct TrapFrame));
	tf->ss = ss;
	tf->esp = KOFFSET - 4;
	tf->eflags = 0x202;
	tf->cs = cs;
	tf->eip = eip;
	tf->ds = ds;
	tf->es = es;
	pcb_table[pcb_index].tf = tf;
	process_ready(pcb_index);
	
	return;
}

void
waitdisk(void) {
	while((in_byte(0x1F7) & 0xC0) != 0X40); /* 等待磁盘完毕 */
}

/* 读磁盘的一个扇区 */
void
readsect(void *dst, int offset) {
	int i;
	waitdisk();
	out_byte(0x1F2, 1);
	out_byte(0x1F3, offset);
	out_byte(0x1F4, offset >> 8);
	out_byte(0x1F5, offset >> 16);
	out_byte(0x1F6, (offset >> 24) | 0xE0);
	out_byte(0x1F7, 0x20);

	waitdisk();
	for (i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = in_long(0x1F0);
	}
}

void
writesect(void *src, int offset) {
	int i;
	waitdisk();
	out_byte(0x1F2, 1);
	out_byte(0x1F3, offset);
	out_byte(0x1F4, offset >> 8);
	out_byte(0x1F5, offset >> 16);
	out_byte(0x1F6, (offset >> 24) | 0xE0);
	out_byte(0x1F7, 0x30);

	waitdisk();
	for (i = 0; i < SECTSIZE / 4; i ++) {
		out_long(0x1F0, ((int *)src)[i]);
	}
}

/* 将位于磁盘offset位置的count字节数据读入物理地址pa */
void
readseg(unsigned char *pa, int count, int offset) {
	unsigned char *epa;
	epa = pa + count;
	pa -= offset % SECTSIZE;
	offset = (offset / SECTSIZE) + 1;
	for(; pa < epa; pa += SECTSIZE, offset ++)
	{
		readsect(pa, offset);
	}
}

void
writeseg(unsigned char *pa, int count, int offset) {
	unsigned char *epa;
	epa = pa + count;
	pa -= offset % SECTSIZE;
	offset = (offset / SECTSIZE) + 1;
	for(; pa < epa; pa += SECTSIZE, offset ++)
	{
		writesect(pa, offset);
	}
}
