#ifndef SYSCALL_H
#define SYSCALL_H

#define PRINT 0
#define GET_TICKS 1

void init_syscall();
int sys_print(char * msg);

#endif