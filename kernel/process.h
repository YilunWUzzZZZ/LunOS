#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "vm.h"
#define NEW 0
#define READY 1
#define RUNNING 2

typedef struct process
{
    uint32 pid;
    uint8 status;
    uint32 esp, ebp;
    uint32 eip;
    pde_t * page_directory;
    struct process * next;
} process_t;

void init_tasking();
void switch_process();
int fork();
int getpid();
void switch_to_user_mode();



#endif