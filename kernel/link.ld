ENTRY(__start)

SECTIONS
{    
    .entry 0x100000:
    {
        kernel/entry_asm.o(.*)
        kernel/entry.o(.*)
    }
    entry_end = .;
    . = 0x80000000 + entry_end;

    .text : AT( ADDR(.text) - 0x80000000)
    {
        code = .; _code = .; __code = .;
        *(.text)
    }
    
    .data ALIGN(4096): 
    {
        data = .; _data = .; __data = .;
        *(.data)
        *(.rodata)
    }

    .bss ALIGN(4096): 
    {
        bss = .; _bss = .; __bss = .;
        *(.bss)
    }

    end = .;  _end = .; __end = .;

}
