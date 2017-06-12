#include "mmupa.h"
#include "memory.h"
#include "cpu.h"

void *memset(void *dst, int c, size_t len);
void *memcpy(void *dst, const void *src, size_t n);

//分配用户页目录表
PDE updir[NR_PDE] align_to_page;
CR3 ucr3;
//inline PDE* get_updir() { return updir; }
PDE* get_updir() { return updir; }	//用于获取用户页目录表

//inline uint32_t get_ucr3() { return ucr3.val; }
uint32_t get_ucr3() { return ucr3.val; }

//inline PDE* get_kpdir();
PDE* get_kpdir();

void init_mm() {
	PDE *kpdir = get_kpdir();

	/* make all PDE invalid */
	memset(updir, 0, NR_PDE * sizeof(PDE));

	/* create the same mapping above 0xc0000000 as the kernel mapping does */
	memcpy(&updir[KOFFSET / PT_SIZE], &kpdir[KOFFSET / PT_SIZE], 
			(PHY_MEM / PT_SIZE) * sizeof(PDE));

	ucr3.val = (uint32_t)va_to_pa((uint32_t)updir) & ~0xfff;
}

