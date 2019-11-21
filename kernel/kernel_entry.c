#include "screen.h"
#include "descriptor_tables.h"
#include "timer.h"
#include "vm.h"
#include "kheap.h"
#include "process.h"
#include "syscall.h"

extern process_t * current_proc;

void main(){
    
    clear_screen();
    print("welcome to LunOS\n");
    init_descriptors();
    init_syscall();
    initialize_paging();
    print("hello paging world\n");
    init_timer(50);
    init_tasking();

    asm volatile("sub $0x300, %esp");
    asm volatile("mov %esp, %ebp");

    asm volatile("sti");
    
    switch_to_user_mode();

    sys_print("Syscall !\n");


    // uint32 * ptr_r = (0x8008FF88 +4);
    // uint32 r;
    // int ret = fork();
    // if(ret)
    // {
    //     print("created a child process, pid: ");
    //     print_dec(ret);
    //     print("\n");
    //     // r = *ptr_r;
    //     // print_hex(r);
    //     // print("\n");

    // }
    // else
    // {
    //     print("hello, i am the child, pid: ");
    //     print_dec(current_proc->pid);
    //     ;
    // }
    
    
    while(1);
}

 __attribute__((__aligned__(PAGESIZE)))
pde_t entrypgdir[PAGESIZE/4] = {
    // Map VA’s [0, 4MB) to PA’s [0, 4MB)
    [0] = (0) | PTE_P | PTE_W | PTE_PS,
    // Map VA’s [KERNBASE, KERNBASE+4MB) to PA’s [0, 4MB)
    [KERNELBASE>>PDX_SHIFT] = (0) | PTE_P | PTE_W | PTE_PS,
};