#include "screen.h"
#include "isr.h"
#include "syscall.h"

void * syscalls[] =
{
    (void *)&print,
};


int sys_print(char * msg)
{
    int a = PRINT;
    int param_num = 1;
    asm volatile("int $0x80":"=a"(a):"0"(a), "b"(param_num));
    return a;
}


uint8 num_syscalls = sizeof(syscalls)/sizeof(void *);

void syscall_handler(registers_t * trap_frame)
{
    uint8 syscall_num = trap_frame->eax;
    if(syscall_num >= num_syscalls)
    {
        trap_frame->eax = -1;
        return;
    }

    uint8 param_num = trap_frame->ebx;
    uint32 user_ebp = trap_frame->ebp;
    print("user ebp: ");
    print_hex(user_ebp);
    print("\n");
    uint32 * str_ptr_addr = user_ebp + 8;
    char * str_ptr = (char *)*(str_ptr_addr) ;
    print(str_ptr);
    int first = user_ebp + 8;
    int i = (param_num-1) * 4 +first;
    
    void * syscall_func = syscalls[syscall_num];
    //push all the paramteter into the stack
    asm volatile("check:\
                  cmp %3, %1;\
                  jl end;    \
                  pushl (%1);\
                  sub $4, %1;\
                  jmp check;\
                  end:\
                  call *%2;":"=a"(trap_frame->eax):"r"(i),"r"(syscall_func), "r"(first));





}

void init_syscall()
{
    register_int_handler((uint32)syscall_handler, 0x80);
}