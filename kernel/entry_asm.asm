[bits 32]
CR4_PSE EQU 0x00000010
KERNBASE EQU 0x80000000
CR0_PG EQU 0x80000000

[global __start]
__start:
[extern init_freelist]
call init_freelist

mov eax, cr4 
or eax, CR4_PSE
mov cr4, eax

[extern entrypgdir]
mov eax, entrypgdir-KERNBASE
mov cr3, eax

mov eax, cr0
or eax, CR0_PG
mov cr0, eax

[extern main]
;jmp tp main
mov ebx, KERNBASE
add esp, ebx
mov eax, main
jmp eax