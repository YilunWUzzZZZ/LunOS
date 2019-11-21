#ifndef  VM_H
#define VM_H
#include "types.h"

#define MAXMEM 0x2000000
#define MAX_KV_MEM 0x40000000
#define CR4_PSE 0x00000010
#define PAGESIZE 4096
#define EXT_PAGESIZE (4096 * 1024)
#define KERNELBASE 0x80000000
#define V2P(addr) ((uint32)(addr) - KERNELBASE)
#define P2V(addr) ((uint32)(addr) + KERNELBASE)
#define PG_ADDR_MASK 0xFFFFF000
#define PG_CTRL_MASK 0xFFF
#define PAGEROUNDUP(addr) ( (uint32)(addr) & 0xFFF ? ( (uint32)(addr) & (~0xFFF) ) + 4096 : (uint32)(addr) )
#define PAGEROUNDDOWN(addr) ((uint32)(addr) & 0xFFFFF000)
#define PD_IDX(addr) ((uint32)(addr)>>22 )
#define PT_IDX(addr) ( ((uint32)(addr) & 0x3FF000) >> 12)
#define PTE_P 0x01
#define PTE_W 0x02
#define PTE_U 0x04
#define PTE_PS 0x080
#define PDX_SHIFT 22
#define PTX_SHIFT 12


//kernel stack for multi-tasking
#define KSTACK_BASE_MT 0xF0000000
#define KSTACK_MAX_SIZE 0x1000 //4k
#define KSTACK_TOP (KSTACK_BASE_MT-KSTACK_MAX_SIZE)

typedef uint32 pde_t;
typedef uint32 pte_t;


struct freemem
{
    struct freemem * next;
};

static inline uint32 v2p(uint32 vaddr)
{
    return vaddr-KERNELBASE;
}

static inline uint32 p2v(uint32 paddr)
{
    return paddr+KERNELBASE;
}

void initialize_paging();
void * kalloc_page();
void kfree_page(struct freemem * address);
uint32 get_phy_addr(uint32 vaddr, pde_t * pgdir);
void switch_pgdir(uint32 new_pgdir);

#endif