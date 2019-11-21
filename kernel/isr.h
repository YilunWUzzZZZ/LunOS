#include "types.h"

#define IRQ0 32


typedef struct registers
{
    uint32 ds;
    uint32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32 int_no, err_code;
    uint32 eip, cs, eflags, useresp, ss;
} registers_t;

typedef void (*handler_t)(registers_t *);

void register_int_handler(uint32, uint8);