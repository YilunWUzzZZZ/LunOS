[bits 32]

[global gdt_flush]
gdt_flush:
    mov eax, [esp+4]
    lgdt [eax]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:flush

flush:
    ret

[global idt_flush]
idt_flush:
    mov eax, [esp+4]
    lidt [eax]
    ret

[global tss_flush]
tss_flush:
    mov ax, 0x28
    ltr ax
    ret

%macro ISR_NO_ERRCODE 1
[global isr%1]
isr%1:
    cli
    push  0
    push  %1
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
[global isr%1]
isr%1:
    cli
    push  %1
    jmp isr_common_stub
%endmacro

%macro IRQ 2
[global irq%1]
irq%1:
    cli
    push 0
    push %2
    jmp irq_common_stub
%endmacro


ISR_NO_ERRCODE 0
ISR_NO_ERRCODE 1
ISR_NO_ERRCODE 2
ISR_NO_ERRCODE 3
ISR_NO_ERRCODE 4
ISR_NO_ERRCODE 5
ISR_NO_ERRCODE 6
ISR_NO_ERRCODE 7
ISR_ERRCODE 8
ISR_NO_ERRCODE 9
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_NO_ERRCODE 15
ISR_NO_ERRCODE 16
ISR_NO_ERRCODE 17
ISR_NO_ERRCODE 18
ISR_NO_ERRCODE 19
ISR_NO_ERRCODE 20
ISR_NO_ERRCODE 21
ISR_NO_ERRCODE 22
ISR_NO_ERRCODE 23
ISR_NO_ERRCODE 24
ISR_NO_ERRCODE 25
ISR_NO_ERRCODE 26
ISR_NO_ERRCODE 27
ISR_NO_ERRCODE 28
ISR_NO_ERRCODE 29
ISR_NO_ERRCODE 30
ISR_NO_ERRCODE 31
ISR_NO_ERRCODE 128
IRQ 0, 32;
IRQ 1, 33;
IRQ 2, 34;
IRQ 3, 35;
IRQ 4, 36;
IRQ 5, 37;
IRQ 6, 38;
IRQ 7, 39;
IRQ 8, 40;
IRQ 9, 41;
IRQ 10, 42;
IRQ 11, 43;
IRQ 12, 44;
IRQ 13, 45;
IRQ 14, 46;
IRQ 15, 47;






[extern isr_handler]
isr_common_stub:
    pusha; edi esi ebp esp ebx edx ecx eax
    
    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_handler

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8
    sti
    iret

[extern irq_handler]
irq_common_stub:
    pusha; edi esi ebp esp ebx edx ecx eax
    
    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call irq_handler

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8
    sti
    iret