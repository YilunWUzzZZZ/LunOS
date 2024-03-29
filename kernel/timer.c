#include "timer.h"
#include "types.h"
#include "screen.h"
#include "isr.h"
#include "utils.h"
#include "process.h"

uint32 tick = 0;

static void timer_callback(registers_t * regs)
{
   tick++;
   switch_process();
   // print("Tick: ");
   // print_dec(tick);
   // print("\n");

}

void init_timer(uint32 frequency)
{
   // Firstly, register our timer callback.
   register_int_handler(timer_callback, IRQ0);

   // The value we send to the PIT is the value to divide it's input clock
   // (1193180 Hz) by, to get our required frequency. Important to note is
   // that the divisor must be small enough to fit into 16-bits.
   uint32 divisor = 1193180 / frequency;

   // Send the command byte.
   outb(0x43, 0x36);

   // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
   uint8 l = (uint8)(divisor & 0xFF);
   uint8 h = (uint8)( (divisor>>8) & 0xFF );

   // Send the frequency divisor.
   outb(0x40, l);
   outb(0x40, h);
}