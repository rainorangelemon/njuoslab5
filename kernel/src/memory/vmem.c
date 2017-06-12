#include "mmupa.h"
#include "memory.h"
#define VMEM_ADDR 0xa0000
#define SCR_SIZE (320 * 200)


int printk(const char *fmt, ...);
void *memset(void *dst, int c, size_t len);
static PTE vdtable[NR_PTE] align_to_page;
/* Use the function to get the start address of user page directory. */

//inline PDE* get_updir();

PDE* get_updir();
void create_video_mapping() {
	/* TODO: create an identical mapping from virtual memory area 
	 * [0xa0000, 0xa0000 + SCR_SIZE) to physical memory area 
	 * [0xa0000, 0xa0000 + SCR_SIZE) for user program. You may define
	 * some page tables to create this mapping.
	 */

	PDE *pdir = (PDE *)get_updir();

	PTE *ptable = vdtable;
		
	pdir->val = make_pde(va_to_pa(vdtable));
	uint32_t pframe_addr =	VMEM_ADDR;
	int i=0;
	for(; i<=0xf; i++) {
		ptable[i+0xa0].val = make_pte(pframe_addr);

		pframe_addr += PAGE_SIZE;
	}
}

