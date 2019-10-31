#include "isr.h"
#include "screen.h"
#include "utils.h"

handler_t interrupt_handlers[256];

void isr_handler(registers_t regs){
    
    print("received interrupt: ");
    print_dec(regs.int_no);
    print("\n");
    print("cs:");
        print_hex(regs.cs);
        print("\n");
        print("ds:");
        print_hex(regs.ds);
        print("\n");
        print("ip:");
        print_hex(regs.eip);
        print("\n");
        print("error code:");
        print_hex(regs.err_code);
        print("\n");
    while(1);
    if(regs.int_no == 13){
       ;
    }


}

void irq_handler(registers_t regs){

    if (regs.int_no >= 40)
    {
       // Send reset signal to slave.
       outb(0xA0, 0x20);
    }
   // Send reset signal to master. (As well as slave, if necessary).
   outb(0x20, 0x20);

   if (interrupt_handlers[regs.int_no] != 0)
   {
       handler_t handler = interrupt_handlers[regs.int_no];
       handler(regs);
   }
}

void register_int_handler(uint32 handler, uint8 num){
    interrupt_handlers[num] = handler;
}