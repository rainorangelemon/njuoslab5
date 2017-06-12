#include "memlayout.h"
#include "x86.h"
#include "pmap.h"
#include "types.h"
#include "assert.h"
#include "pcb.h"
#include "pcb_struct.h"
#include "cpu.h"

#define NR_PDE 	1024
#define NR_PTE  1024
#define PAGE_SIZE 4096
/*#define PT_SIZE   ((NR_PTE) * (PAGE_SIZE))*/
#define KOFFSET   0xC0000000
#define PHY_MEM   (128 * 1024 * 1024)
int printk(const char *fmt, ...);
pde_t* get_updir();
void *	memcpy(void *dst, const void *src, size_t len);
extern void *memset(void *v, int c, size_t n);
extern struct PCB *current;

enum {
	//Kernel error codes -- keep in sync with list in lib/printfmt.c.
	E_UNSPECIFIED   = 1,    // Unspecified or unknown problem
	E_BAD_ENV       = 2,    // Environment doesn't exist or otherwise
	                        // cannot be used in requested action
	E_INVAL         = 3,    // Invalid parameter
	E_NO_MEM        = 4,    // Request failed due to memory shortage
	E_NO_FREE_ENV   = 5,    // Attempt to create a new environment beyond
				// the maximum allowed
	E_FAULT         = 6,    // Memory fault
	E_NO_SYS        = 7,    // Unimplemented system call

	MAXERROR
};

#define npages (1<<15)			// Amount of physical memory (in pages)

struct PageInfo pages[npages];		// Physical page state array
static struct PageInfo *page_free_list;	// Free list of physical pages


// --------------------------------------------------------------
// Tracking of physical pages.
// The 'pages' array has one 'struct PageInfo' entry per physical page.
// Pages are reference counted, and free pages are kept on a linked list.
// --------------------------------------------------------------

void
page_init(void)
{
	// The example code here marks all physical pages as free.
	// However this is not truly the case.  What memory is free?
	// 
	// NB: DO NOT actually touch the physical memory corresponding to
	// free pages!
	unsigned long i;
	for (i = 0; i < npages; i++) {
		if(i > 1024)
		{
			
			pages[i].pp_ref = 0;
			pages[i].pp_link = page_free_list;
			page_free_list = &pages[i];
		}
		else
		{
			pages[i].pp_ref = 1;
			pages[i].pp_link = NULL;
		}
	}
}

//
// Allocates a physical page.  If (alloc_flags & ALLOC_ZERO), fills the entire
// returned physical page with '\0' bytes.  Does NOT increment the reference
/// count of the page - the caller must do these if necessary (either explicitly
// or via page_insert).
//
// Be sure to set the pp_link field of the allocated page to NULL so
// page_free can check for double-free bugs.
//
// Returns NULL if out of free memory.
//
// Hint: use page2kva and memset
struct PageInfo *
page_alloc(int alloc_flags)
{
	// Fill this function in
	if(page_free_list == NULL)
	{
		return NULL;
	}
	
	struct PageInfo *pg_alloc = page_free_list;
	page_free_list = pg_alloc->pp_link;
	pg_alloc->pp_link = NULL;
	pg_alloc->pp_ref = 0;

	if(alloc_flags & ALLOC_ZERO)
	{
		memset(page2kva(pg_alloc), 0, PGSIZE);
	}
	return pg_alloc;
}

//
// Return a page to the free list.
// (This function should only be called when pp->pp_ref reaches 0.)
//
void
page_free(struct PageInfo *pp)
{
	// Fill this function in
	// Hint: You may want to panic if pp->pp_ref is nonzero or
	// pp->pp_link is not NULL.

	//	if(pp->pp_ref != 0 || pp->pp_link != NULL)
//		panic("ERROR to free a nonfree page!");
	pp->pp_link = page_free_list;
	page_free_list = pp;
}

//
// Decrement the reference count on a page,
// freeing it if there are no more refs.
//
void
page_decref(struct PageInfo* pp)
{
	if (--pp->pp_ref == 0)
		page_free(pp);
}

// Given 'pgdir', a pointer to a page directory, pgdir_walk returns
// a pointer to the page table entry (PTE) for linear address 'va'.
// This requires walking the two-level page table structure.
//
// The relevant page table page might not exist yet.
// If this is true, and create == false, then pgdir_walk returns NULL.
// Otherwise, pgdir_walk allocates a new page table page with page_alloc.
//    - If the allocation fails, pgdir_walk returns NULL.
//    - Otherwise, the new page's reference count is incremented,
//	the page is cleared,
//	and pgdir_walk returns a pointer into the new page table page.
//
// Hint 1: you can turn a Page * into the physical address of the
// page it refers to with page2pa() from pmap.h.
//
// Hint 2: the x86 MMU checks permission bits in both the page directory
// and the page table, so it's safe to leave permissions in the page
// directory more permissive than strictly necessary.
//
// Hint 3: look at inc/mmu.h for useful macros that mainipulate page
// table and page directory entries.
//
pte_t *
pgdir_walk(pde_t *pgdir, const void *va, int create)
{
	// Fill this function in
	pde_t *pde = &pgdir[PDX(va)];
	pte_t *pte;
	struct PageInfo *pginfo;
	
	if(!create)
	{
		if(!(*pde & PTE_P))
			return NULL;
		else
			pte = (pte_t *)(PTE_ADDR(*pde) + PTX(va) * 4);
		if(!(*pte & PTE_P))
			return NULL;
		else
			return pte;
	}
	else
	{
		if(!(*pde & PTE_P))
		{
			pginfo = page_alloc(ALLOC_ZERO);
			pginfo->pp_ref++;
			*pde = page2pa(pginfo) | 0x7;
		}

		pte = (pte_t *)(PTE_ADDR(*pde) + PTX(va) *4);

		if(!(*pte & PTE_P))
		{
			pginfo = page_alloc(ALLOC_ZERO);
			pginfo->pp_ref++;
			*pte = page2pa(pginfo);
			*pte = *pte | 0x7;
		}

		return pte;
	}
}

uint32_t mm_malloc(uint32_t va, int len)
{
	//用户页目录表在别处定义，这里通过函数去获取页目录表首地址
	pte_t *pte = pgdir_walk(get_updir(), (void*)va, 1);
	return PTE_ADDR(*pte) + PGOFF(va);
}

//----------------------------------------------------
/*首先要分配一个物理页来作为子进程的页目录
 *然后遍历父进程的页目录，如果父进程页目录表项有内容
 ×则相应的要给子进程的该表项填写内容
 *然后遍历页目录，填写对应的页目录项
 *复制物理页
 *对于页目录的0xC0000000以上的地方，直接拷贝即可
 *另外，还要分配用户栈，做现存映射
 *填写pcb的cr3
 */
//----------------------------------------------------
void copy(int pcb_index)
{
	//分配物理页作为页目录，并填写pcb的cr3
	struct PageInfo *pgdir = page_alloc(ALLOC_ZERO);
	pgdir->pp_ref++;
	CR3 cr3;
	cr3.val = 0;
	cr3.page_directory_base = page2pa(pgdir) >> PGSHIFT;	//!!!
	pcb_cr3write(pcb_index, cr3.val);

	//直接复制0xC0000000往上的页目录项
	pde_t *child = (pde_t *)page2kva(pgdir);
	pde_t *father = (pde_t *) (current->cr3 + 0xc0000000);
	memcpy(&child[KOFFSET / PT_SIZE], &father[KOFFSET / PT_SIZE], (PHY_MEM / PT_SIZE) * sizeof(PDE));
	
	//遍历父进程页目录
	for(int i = 0; i < KOFFSET /PT_SIZE; i ++)
	{
		if((*(father+i) & PTE_P))
		{	//分配页表一张
			struct PageInfo *pte = page_alloc(ALLOC_ZERO);
			pte->pp_ref++;
			//填写页目录项
			*(child+i) = page2pa(pte) | 0X7;
			//指向页目录首地址
			pte_t *child_t = (pte_t *)(PTE_ADDR(*(child+i)) + KOFFSET);
			pte_t *father_t = (pte_t *)(PTE_ADDR(*(father+i)) + KOFFSET);
			for(int j = 0; j < NR_PTE; j ++)
			{
				if((*(father_t+j) & PTE_P))
				{
					struct PageInfo *pp = page_alloc(ALLOC_ZERO);
					pp->pp_ref++;

					*(child_t+j) = page2pa(pp) | 0x7;

					memcpy((void *)(PTE_ADDR(*(child_t+j)) + KOFFSET), (void*)(PTE_ADDR(*(father_t+j)) + KOFFSET), PGSIZE);		//!!!
				}
			}
		}
	}
	
}

/**************************************************************/
void thread_copy(int pcb_index)
{
	struct PageInfo *pgdir = page_alloc(ALLOC_ZERO);
	pgdir->pp_ref++;
	CR3 cr3;
	cr3.val = 0;
	cr3.page_directory_base = page2pa(pgdir) >> PGSHIFT;
	pcb_cr3write(pcb_index, cr3.val);

	//复制页目录的所有内容
	pde_t *child = (pde_t *)page2kva(pgdir);
	pde_t *father = (pde_t *)(current->cr3 + 0xc0000000);
	memcpy(&child[0], &father[0], 4 * (1 << 10));
	
	//修改和内核栈相关的内容
	int index = (KOFFSET - PT_SIZE) / PT_SIZE;

	//用户栈对应的页表
	struct PageInfo *pte = page_alloc(ALLOC_ZERO);
	pte->pp_ref++;
	*(child+index) = page2pa(pte) | 0x7;
	
	//给页表分配物理页
	pte_t *child_t = (pte_t *)(PTE_ADDR(*(child+index)) + KOFFSET);
	for(int j = 0; j < NR_PTE; j ++)
	{
		struct PageInfo *pp = page_alloc(ALLOC_ZERO);
		pp->pp_ref ++;
		*(child_t+j) = page2pa(pp) | 0x7;
	}
}

void release()
{
	pde_t *pgdir = (pde_t *) (current->cr3 + 0xc0000000);
	for(int i = 0; i < KOFFSET /PT_SIZE; i ++)
	{
		if((*(pgdir+i) & PTE_P))
		{
			pte_t *pte = (pte_t *)(PTE_ADDR(*(pgdir+i)) + KOFFSET);
			for(int j = 0; j < NR_PTE; j ++)
			{
				if((*(pte+j) & PTE_P))
				{
					uint32_t addr = *(pte+j);
					struct PageInfo* pp = pa2page(addr);
					pp->pp_ref = 0;
					page_free(pp);
				}
			}
			uint32_t taddr = *(pgdir+i);
			struct PageInfo* tpp = pa2page(taddr);
			tpp->pp_ref = 0;
			page_free(tpp);
		}
	}
	
	CR3 cr3;
	cr3.val = current->cr3;


	uint32_t paddr = cr3.page_directory_base << PGSHIFT;
	struct PageInfo* pgpp = pa2page(paddr);
	pgpp->pp_ref = 0;
	page_free(pgpp);
}




//
// Return the page mapped at virtual address 'va'.
// If pte_store is not zero, then we store in it the address
// of the pte for this page.  This is used by page_remove and
// can be used to verify page permissions for syscall arguments,
// but should not be used by most callers.
//
// Return NULL if there is no page mapped at va.
//
// Hint: the TA solution uses pgdir_walk and pa2page.
//
struct PageInfo *
page_lookup(pde_t *pgdir, void *va, pte_t **pte_store)
{
	// Fill this function in
	pte_t *pte = pgdir_walk(pgdir, va, 0);
	if(pte == NULL)
		return NULL;
	
	if(!pte_store)
		*pte_store = pte;
	return pa2page(PTE_ADDR(*pte));
}

//
// Unmaps the physical page at virtual address 'va'.
// If there is no physical page at that address, silently does nothing.
//
// Details:
//   - The ref count on the physical page should decrement.
//   - The physical page should be freed if the refcount reaches 0.
//   - The pg table entry corresponding to 'va' should be set to 0.
//     (if such a PTE exists)
//   - The TLB must be invalidated if you remove an entry from
//     the page table.
//
// Hint: The TA solution is implemented using page_lookup,
// 	tlb_invalidate, and page_decref.
//
void
page_remove(pde_t *pgdir, void *va)
{
	// Fill this function in
	pte_t *pte;

	pte_t **pte_store = &pte;
	struct PageInfo *pginfo = page_lookup(pgdir, va, pte_store);
	if(pginfo == NULL)
		return;

	page_decref(pginfo);
	**pte_store = 0;
	tlb_invalidate(pgdir, va);
}

//
// Invalidate a TLB entry, but only if the page tables being
// edited are the ones currently in use by the processor.
//
void
tlb_invalidate(pde_t *pgdir, void *va)
{
	// Flush the entry only if we're modifying the current address space.
	// For now, there is only one address space, so always invalidate.
	invlpg(va);
}
