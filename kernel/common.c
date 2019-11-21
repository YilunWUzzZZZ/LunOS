#include "common.h"
#include "screen.h"

void panic(char * msg){
    print(msg);
    while(1);
}

void panic_assert(char * msg)
{
    print("ASSERT failed: ");
    print(msg);
    print("\n");
    for(;;);
}