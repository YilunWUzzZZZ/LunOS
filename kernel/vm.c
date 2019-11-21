#include "vm.h"
#include "utils.h"
#include "isr.h"
#include "screen.h"
#include "common.h"
#include "kheap.h"

extern uint32 end;
struct freemem * freelist;
pde_t * kernel_pgdir;
extern pde_t * current_pgdir;
extern uint32 placement_address;
extern heap_t * kheap;



void page_fault(registers_t * registers);

void * kalloc_page()
{
    if(freelist)
    {
        struct freemem * free_mem = freelist;
        freelist = (struct freemem *)P2V(freelist->next);
        return (void *)free_mem;
    }

    else
    {
        panic("ran out of free memory\n");
    }
}

void kfree_page(struct freemem * address)
{
    address =  (struct freemem *)((uint32)address & (~0xFFF));
    memset((sint8 *)address, 0, PAGESIZE);
    address->next = (struct freemem *)V2P(freelist);
    freelist = address;
}

uint32 get_phy_addr(uint32 vaddr, pde_t * pgdir)
{
    pde_t * pgtbl = pgdir[PD_IDX(vaddr)] & 0xFFFFF000;
    uint32 phy_addr = pgtbl[PT_IDX(vaddr)] & 0xFFFFF000;
    return phy_addr;
}

void map_page(uint32 vaddr, uint32 paddr, pde_t * kernel_pgdir, uint32 control)
{
    
    pte_t * pg_table = kernel_pgdir[vaddr >> PDX_SHIFT] & (~0xFFF);
    if(!pg_table)
    {
        pg_table = (pte_t *)kmalloc(PAGESIZE, 1);
        memset(pg_table, 0, PAGESIZE);
        kernel_pgdir[PD_IDX(vaddr)] = V2P(pg_table)| control;
        
    }
    
    pg_table[PT_IDX(vaddr)] = (paddr&(~0xFFF) ) | control;

}

void map_page_4mb(uint32 vaddr, uint32 paddr, pde_t * kernel_pgdir, uint32 control)
{
    
    paddr = paddr & (~0x003FFFFF);

    kernel_pgdir[vaddr >> PDX_SHIFT] = paddr | control | PTE_PS;

}

void switch_pgdir(uint32 new_pgdir){

    asm volatile("movl %0, %%cr3"::"r"(new_pgdir));

}

void initialize_paging(){

    kernel_pgdir = (pde_t *)kmalloc(PAGESIZE, 1);
    memset(kernel_pgdir, 0, PAGESIZE);

    //map all the physical memory 
    for(uint32 i= 0; i<MAXMEM; i+=PAGESIZE)
    {
        map_page(i+KERNELBASE, i, kernel_pgdir, PTE_P|PTE_W|PTE_U);

    }
    //map the heap
    for(uint32 i= KHEAP_START; i<KHEAP_START+KHEAP_INITIAL_SIZE; i+=PAGESIZE)
    {
        uint32 paddr = V2P(kmalloc(PAGESIZE, 1));
        map_page(i, paddr, kernel_pgdir, PTE_P|PTE_W|PTE_U); 

    }
    //map the new kernel stack
    for(uint32 i= KSTACK_BASE_MT-KSTACK_MAX_SIZE; i<KSTACK_BASE_MT; i+=PAGESIZE)
    {
        uint32 paddr = V2P(kmalloc(PAGESIZE, 1));
        map_page(i, paddr, kernel_pgdir, PTE_P|PTE_W|PTE_U); 
    }

    register_int_handler((uint32)page_fault, 14);

    current_pgdir = kernel_pgdir;
    switch_pgdir(V2P(kernel_pgdir));

    kheap = create_heap(KHEAP_START, KHEAP_START+KHEAP_INITIAL_SIZE, KHEAP_MAX, 1, 0);

    freelist = PAGEROUNDUP(placement_address);
    
}



void page_fault(registers_t * registers){
    uint32 fault_addr;
    asm volatile("movl %%cr2, %0":"=r"(fault_addr));
    print("fault address: ");
    print_hex(fault_addr);
    print("\n");
    panic("page fault!\n");
    
}