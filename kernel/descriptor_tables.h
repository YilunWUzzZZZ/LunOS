#include "types.h"
#define USER_CODE_ACCESS 0xFA  //11111010b
#define SYS_CODE_ACCESS  0x9A  //10011010b
#define USER_DATA_ACCESS 0xF2  //11110010b
#define SYS_DATA_ACCESS  0x92  //10010010b
#define GRAN 0xCF  //1100FFFFb

struct gdt_entry_struct
{
    uint16 limit_0_15;
    uint16 base_0_15;
    uint8  base_16_23;
    uint8  access; //P DPL DT TYPE(CODE/DATA Conforming/Expanddown  R/W ACCESSED)
    uint8  granularity; //G D 0 A limit16-19
    uint8  base_24_31;
} __attribute__((packed));

typedef struct gdt_entry_struct gdt_entry_t;


struct gdt_ptr_struct
{
    uint16 limit;
    uint32 base;
} __attribute__((packed));

typedef struct gdt_ptr_struct gdt_ptr_t;

struct idt_entry_struct
{
    uint16 base_0_15;
    uint16 sel;
    uint8 always0;
    uint8 flags;
    uint16 base_16_31;
}__attribute__((packed));

typedef struct idt_entry_struct idt_entry_t;

struct idt_ptr_struct
{
    uint16 limit;
    uint32 base;
} __attribute__((packed));

typedef struct idt_ptr_struct idt_ptr_t;

struct tss_entry_struct
{
   uint32 prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
   uint32 esp0;       // The stack pointer to load when we change to kernel mode.
   uint32 ss0;        // The stack segment to load when we change to kernel mode.
   uint32 esp1;       // Unused...
   uint32 ss1;
   uint32 esp2;
   uint32 ss2;
   uint32 cr3;
   uint32 eip;
   uint32 eflags;
   uint32 eax;
   uint32 ecx;
   uint32 edx;
   uint32 ebx;
   uint32 esp;
   uint32 ebp;
   uint32 esi;
   uint32 edi;
   uint32 es;         // The value to load into ES when we change to kernel mode.
   uint32 cs;         // The value to load into CS when we change to kernel mode.
   uint32 ss;         // The value to load into SS when we change to kernel mode.
   uint32 ds;         // The value to load into DS when we change to kernel mode.
   uint32 fs;         // The value to load into FS when we change to kernel mode.
   uint32 gs;         // The value to load into GS when we change to kernel mode.
   uint32 ldt;        // Unused...
   uint32 trap;
   uint32 iomap_base;
} __attribute__((packed));

typedef struct tss_entry_struct tss_entry_t;

void init_descriptors();
void init_idt();
void set_kernel_stack(uint32 stack);

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr128();


extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();