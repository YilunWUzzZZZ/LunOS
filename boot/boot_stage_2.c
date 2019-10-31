#include "../kernel/types.h"
#include "../kernel/utils.h"
#include "../kernel/elf.h"

#define KERNEL_START_SEC 3
#define ELF_MAGIC 0x464C457FU
#define KERNEL_BASE 0x100000

void waitdisk(void){

    while(inb(0x1F7) & 0xC0 != 0x40){
        ; //spin
    }
}

//read the "offset" section to "dest"  
void readsect(void * dest, uint32 offset){
    waitdisk();
    outb(0x1F2, 1);// count
    outb(0x1F3, offset);
    outb(0x1F4, offset >> 8);
    outb(0x1F5, offset >> 16);
    outb(0x1F6, (offset >> 24) | 0xE0);
    outb(0x1F7, 0x20); //read cmd

    waitdisk();
    insl(0x1F0, dest, SECTSIZE/4);
}

//read count bytes starting at offset from kernel to pa
void readseg(uint8 * pa, uint32 count, uint32 offset){
    uint8 * epa = pa + count;
    pa -= offset % SECTSIZE;

    offset = (offset / SECTSIZE) + KERNEL_START_SEC;
    for(; pa < epa; pa += SECTSIZE, offset++){
        readsect(pa, offset);
    }
}

void loadkernel(){
    char * video_mem = 0xb8000;
    
    elf_header_t * elfhdr = 0x30000; //start address for elf header

    void (*kernel_entry)(void);
    readseg(elfhdr, 4096, 0);

    if(elfhdr->magic != ELF_MAGIC){
        return;
    }
    
    prog_header_t * pghdr = (uint8*)elfhdr + elfhdr->phoff;
    int phnum = elfhdr->phnum;
    for(int i=0; i<phnum; i++, pghdr++){
        uint8  * pa = pghdr->paddr;
        readseg(pa, pghdr->filesz, pghdr->off);
        if(pghdr->filesz < pghdr->memsz){
            stosb(pa + pghdr->filesz, 0, pghdr->memsz - pghdr->filesz);
        }
    }
    kernel_entry = (void(*)(void)) (elfhdr->entry);
    kernel_entry();
}