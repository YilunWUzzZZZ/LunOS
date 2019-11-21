#include "vm.h"
#include "kheap.h"
#include "utils.h"
#include "process.h"
#include "screen.h"
#include "common.h"
#include "descriptor_tables.h"

#define KSTACK_START (0x90000+KERNELBASE)

extern pde_t * kernel_pgdir;
pde_t * current_pgdir;
volatile process_t * current_proc;
volatile process_t * ready_queue;
extern uint32 read_eip();
uint32 next_pid = 1;

uint32 clone_page_table(pte_t * src);
void copy_kernel_stack(pde_t * pgdir);

void switch_to_user_mode()
{
   // Set up a stack structure for switching to user mode.
   asm volatile("  \
     cli; \
     mov $0x23, %ax; \
     mov %ax, %ds; \
     mov %ax, %es; \
     mov %ax, %fs; \
     mov %ax, %gs; \
                   \
     mov %esp, %eax; \
     pushl $0x23; \
     pushl %eax; \
     pushf; \
     pop %eax; \
     or $0x200, %eax;\
     push %eax;\
     pushl $0x1B; \
     push $1f; \
     iret; \
   1: \
     ");
}




pde_t * clone_page_dir(pde_t * src)
{
    pde_t * new_pgdir = (pde_t *)kalloc_page();
    memset((sint8*)new_pgdir, 0, PAGESIZE); //clear the page directory

    //link the kernel 
    for(uint32 i=KERNELBASE; i != 0; i += PAGESIZE*1024)
    {
        uint16   pd_idx = PD_IDX(i);
        new_pgdir[pd_idx] = src[pd_idx];
    }

    //we need a new kernel stack
    copy_kernel_stack(new_pgdir);

    for(uint32 i=0; i<KERNELBASE; i += PAGESIZE*1024)
    {
        uint16   pd_idx = PD_IDX(i);

        if(src[pd_idx])//if mapped
        {
            //set the new page directory entry
            uint32 table_paddr = P2V(src[pd_idx] & 0xFFFFF000);
            
            new_pgdir[pd_idx] = clone_page_table(table_paddr);//set up a new page table and copy the physical frames
            new_pgdir[pd_idx] |= src[pd_idx] & 0xFFF; // copy the control bits
        }
    }

    return new_pgdir;// its virtual address

}

uint32 copy_phy_frame(uint32 src)
{
    uint32 new_frame = (uint32)kalloc_page();
    memcpy(new_frame, src, PAGESIZE);
    return V2P(new_frame);
}

uint32 clone_page_table(pte_t * src)
{
    pte_t * new_pgtbl =  (pde_t *)kalloc_page();;
    memset((sint8*)new_pgtbl, 0, PAGESIZE);
    
    for(uint16 i=0; i < PAGESIZE/4; i++)
    {

        if(src[i])
        {   
            //copy physical frame
            uint32 src_page_addr = P2V(src[i] & 0xFFFFF000);//virtual address
            uint32 new_page_addr = (uint32)kalloc_page();
            memcpy((uint8*)new_page_addr, (uint8*)src_page_addr, PAGESIZE);

            //set the page table
            new_pgtbl[i] = V2P(new_page_addr) | (src[i] & 0xFFF);
        }
    }

    return V2P(new_pgtbl);
}

void copy_kernel_stack(pde_t * pgdir)
{
     pte_t * new_pgtbl;
     for(uint32 vaddr=KSTACK_TOP; vaddr < KSTACK_BASE_MT; vaddr+=PAGESIZE)
     {
         pte_t * pgtbl = (pte_t *)P2V((pgdir[PD_IDX(vaddr)] & PG_ADDR_MASK));//old page table
         
         if(vaddr & 0x3FFFFF == 0 || vaddr == KSTACK_TOP)
         {
            new_pgtbl = (pte_t * )kalloc_page();//create a new page table
            pgdir[PD_IDX(vaddr)] = V2P(new_pgtbl) | (pgdir[PD_IDX(vaddr)] & PG_CTRL_MASK);
            memcpy((uint8 *)new_pgtbl, (uint8 *)pgtbl, PAGESIZE);
         }
         
         uint16 pt_idx = PT_IDX(vaddr);
         uint32 src_frame =  P2V(pgtbl[pt_idx] & PG_ADDR_MASK);
         new_pgtbl[pt_idx]= copy_phy_frame(src_frame) | (pgtbl[pt_idx] & PG_CTRL_MASK);
     }
}

void move_kernel_stack()
{
    uint32 esp, ebp;
    asm volatile("mov %%esp, %0":"=r"(esp));
    asm volatile("mov %%ebp, %0":"=r"(ebp));

    uint32 size = KSTACK_START - esp;
    uint32 dest = KSTACK_BASE_MT - size;

    
    if(KSTACK_START - esp > KSTACK_MAX_SIZE)
    {
        //we have a problem
        panic("defined stack size too small");
    }

    memcpy((uint8*)dest, (uint8*)esp, size);//copy it
    //change all the base pointer value
    uint32 offset = KSTACK_BASE_MT - KSTACK_START;
    for(uint32 * i = dest; (uint32)i < KSTACK_BASE_MT; i++)
    {
        if(esp <= (*i) && (*i) <= KSTACK_START)
        {
            *i  = *i + offset;
        } 
    }
    esp += offset;
    ebp += offset;

    asm volatile("mov %0, %%esp"::"r"(esp));
    asm volatile("mov %0, %%ebp"::"r"(ebp));
}

void init_tasking()
{
    asm volatile("cli");
    
    move_kernel_stack();
    current_proc = (process_t *)kmalloc(sizeof(process_t), 0);
    ready_queue = current_proc;
    current_proc->pid = next_pid++;
    current_proc->esp = 0;
    current_proc->ebp=0;
    current_proc->eip =0;
    current_proc->page_directory = kernel_pgdir;
    current_proc->next = 0;
    current_proc->status = RUNNING;
    asm volatile("sti");  

}


int fork()
{
    asm volatile("cli");
    process_t * parent = (process_t *)current_proc;
    pde_t * new_pgdir = clone_page_dir(current_pgdir);

    

    process_t * new_proc = (process_t * )kmalloc(sizeof(process_t), 0);
    new_proc->pid = next_pid++;
    new_proc->page_directory = new_pgdir;
    new_proc->next = 0;
    new_proc->status = NEW;

    process_t * temp_proc =  (process_t *)ready_queue;
    while(temp_proc->next)
    {
        temp_proc = temp_proc->next;
    }

    temp_proc->next = new_proc;

    uint32 eip = read_eip();

    if(current_proc == parent)
    {
        uint32 esp, ebp;
        
        asm volatile("mov %%esp, %0":"=r"(esp));
        asm volatile("mov %%ebp, %0":"=r"(ebp));
        new_proc->esp = esp;
        new_proc->ebp = ebp;
        new_proc->eip = eip;
        asm volatile("sti");
        print("in fork, parent\n");
        print_hex(eip);
        print(" ");
        print_hex(esp);
        print(" ");
        print_hex(ebp);
        uint32 ret_a;
        asm volatile("mov 0x4(%%ebp), %0":"=r"(ret_a));
        print(" ");
        print_hex(ret_a);
        print("\n");
        return new_proc->pid;
    }
    else
    {
        uint32 ret_a, ebp;
        asm volatile("mov 0x4(%%ebp), %0":"=r"(ret_a));
        asm volatile("mov %%ebp, %0":"=r"(ebp));
        print("in fork\n");
        print_dec(current_proc->pid);
        print(" ");
        print_hex(ebp);
        print(" ");
        print_hex(ret_a);
        print("\n");
        return 0; //child
    }
    


}

void switch_process()
{
    if(!current_proc)
        return;

    uint32 esp, ebp, eip;
    asm volatile("mov %%esp, %0":"=r"(esp));
    asm volatile("mov %%ebp, %0":"=r"(ebp));

    eip = read_eip();

    if(eip == 0x12345)
    {
        // print("returning from the switch\n");
        // uint32 ret_a;
        // asm volatile("mov 0x4(%%ebp), %0":"=r"(ret_a));
        // print_dec(current_proc->pid);
        // print(" ");
        // print_hex(ret_a);
        // print("\n");
        return;
    }
    current_proc->eip = eip;
    current_proc->esp = esp;
    current_proc->ebp = ebp;
    current_proc->status = READY;

    current_proc = (current_proc->next)?(current_proc->next):ready_queue;
    current_pgdir = current_proc->page_directory;
    esp = current_proc->esp;
    ebp = current_proc->ebp;
    // print("jumping to:\n");
    // print_dec(current_proc->pid);
    // print(" ");
    // print_hex(eip);
    // print(" ");
    // print_hex(esp);
    // print(" ");
    // print_hex(ebp);
    // print("\n");

    if(current_proc->status == NEW)
    {
        eip = current_proc->eip;
    }
    current_proc->status = RUNNING;


    asm volatile("      \
        cli;            \
        mov %0, %%ecx;  \
        mov %1, %%esp;  \
        mov %2, %%ebp;  \
        mov %3, %%cr3;  \
        mov $0x12345, %%eax;\
        sti;            \
        jmp *%%ecx;  ": : "r"(eip), "r"(esp), "r"(ebp), "r"(V2P(current_pgdir)): "%ecx");
}
