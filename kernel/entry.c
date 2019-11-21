#include "types.h"
#include "vm.h"

extern uint32 end;

void init_freelist(){


    for(uint32 i= PAGEROUNDUP(V2P(&end)); i<MAXMEM; i+=PAGESIZE){
        ((struct freemem *)i)->next = (struct freemem *)(i+PAGESIZE);
    }
    
}

