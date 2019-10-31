#include "screen.h"
#include "descriptor_tables.h"
#include "timer.h"

void main(){
    
    clear_screen();
    print("welcome to ExOS\n");
    init_descriptors();
    init_timer(50);
    asm volatile("sti");
    while(1);
}