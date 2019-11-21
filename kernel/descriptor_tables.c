#include "descriptor_tables.h"
#include "utils.h"
#include "vm.h"
#define GDT_NUM 6

gdt_entry_t gdt[GDT_NUM];
gdt_ptr_t gdt_ptr;
idt_entry_t idt[256];
idt_ptr_t idt_ptr;
tss_entry_t tss_entry;

void (*isr_func[])(void) = {isr0,isr1,isr2,isr3,isr4,isr5,isr6,isr7,isr8,isr9,isr10,isr11,isr12,isr13,
isr14,isr15,isr16,isr17,isr18,isr19,isr20,isr21,isr22,isr23,isr24,isr25,isr26,isr27,isr28,isr29,isr30,isr31};

void (*irq_func[])(void) = {irq0,irq1,irq2,irq3,irq4,irq5,irq6,irq7,irq8,irq9,irq10,irq11,irq12,irq13,irq14,irq15};


extern void gdt_flush(uint32);
extern void idt_flush(uint32);
extern void tss_flush();

static void gdt_set_gate(uint32 num, uint32 limit, uint32 base, uint8 access, uint8 granularity){
    gdt[num].base_0_15 = (uint16)base;
    gdt[num].base_16_23 = (uint8)(base >> 16);
    gdt[num].base_24_31 = (uint8)(base >> 24);
    gdt[num].limit_0_15 = (uint16)limit;
    gdt[num].granularity = (granularity << 4) + ( 0x0F & (limit >> 16) );
    gdt[num].access = access;

}

static void write_tss(uint32 num, uint16 ss0, uint32 esp0)
{
    uint32 base = (uint32)&tss_entry;
    uint32 limit = sizeof(tss_entry)-1;
    gdt_set_gate(num, limit, base, 0x89, 0x00);

    memset(&tss_entry, 0, sizeof(tss_entry));

    tss_entry.ss0 = ss0;
    tss_entry.esp0 = esp0;
    tss_entry.cs = 0x0b;//1011b CPL=3
    tss_entry.ss = tss_entry.ds = tss_entry.es =  tss_entry.fs = tss_entry.gs = 0x13;
}

void set_kernel_stack(uint32 stack)
{
    tss_entry.esp0 = stack;
}

static void init_gdt(){
    gdt_ptr.limit = sizeof(gdt_entry_t)*GDT_NUM - 1;
    gdt_ptr.base = (uint32)gdt;

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0x0FFFFF, 0, SYS_CODE_ACCESS, GRAN);
    gdt_set_gate(2, 0x0FFFFF, 0, SYS_DATA_ACCESS, GRAN);
    gdt_set_gate(3, 0x0FFFFF, 0, USER_CODE_ACCESS, GRAN);
    gdt_set_gate(4, 0x0FFFFF, 0, USER_DATA_ACCESS, GRAN);
    write_tss(5, 0x10, KSTACK_BASE_MT);
    gdt_flush((uint32)(&gdt_ptr));
    tss_flush();

}

static void idt_set_gate(uint8 num, uint32 base, uint16 sel, uint8 flags){
    idt[num].base_0_15 = (uint16)base;
    idt[num].base_16_31 = (uint16)(base>>16);
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void init_idt(){
    idt_ptr.limit = sizeof(idt_entry_t)*256 -1;
    memset(idt, 0, sizeof(idt_entry_t)*256);
    idt_ptr.base = idt;

    // Remap the irq table.
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    for(int i=0; i<32; i++){
        idt_set_gate(i, (uint32)isr_func[i], 0x08, 0x8E);
    }
    for(int j=0; j<16; j++){
        idt_set_gate(j+32, (uint32)irq_func[j], 0x08, 0x8E);
    }
    //set the syscall gate
    idt_set_gate(0x80, (uint32)isr128, 0x08, 0xEF);

    idt_flush((uint32)(&idt_ptr));

}

void init_descriptors(){
    init_gdt();
    init_idt();
}

