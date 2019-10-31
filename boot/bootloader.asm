[org 0x7c00]

STAGE_2_OFFSET equ 0x1000

mov bp, 0x9000
mov sp, bp
mov bx, MSG_Hello
mov [BOOT_DRIVE], dl
call print_str
call load_stage_2
call switch_to_pm


print_str:
    push ax
    mov ah, 0x0e
print:   
    mov al, [bx]
    cmp al, 0
    je  exit
    int 0x10
    inc bx
    jmp print
exit:
    pop ax
    ret

; read dh sectors in drive dl to es:bx  
disk_loader:
    push dx
    mov ah, 0x02
    mov al, dh
    mov dh, 0
    mov ch, 0x0
    mov cl, 2

    int 0x13

    jc disk_error

    pop dx
    cmp dh, al
    jne disk_error
    ret

disk_error:
    mov bx, MSG_DISK_ERR
    call print_str
    jmp $ ;Hang


load_stage_2:
    mov bx, MSG_LOAD_STAGE_2
    call print_str
    mov bx, STAGE_2_OFFSET
    mov dl, [BOOT_DRIVE]
    mov dh, 2
    call disk_loader
    ret


[bits 32]
VIDEO_MEM equ 0xb8000
WHITE_ON_BLACK equ 0x0f

print_str_pm:
    pusha
    mov edx, VIDEO_MEM
    mov ah,  WHITE_ON_BLACK
print_pm:
    mov al, [ebx]
    cmp al, 0
    jz  print_pm_return
    mov [edx], ax
    add edx, 2
    inc ebx
    jmp print_pm
print_pm_return:
    popa
    ret

gdt_start:

gdt_null:
    dd 0x0
    dd 0x0
gdt_code_seg:
    dw 0xffff   ;Limit(0-15)
    dw 0x0      ;Base (0-15)
    db 0x0      ;Base (16-23)
    db 10011010b;(present) 1 (privilege)00 (dscpt type) 1
                ;(code) 1 (conform)0 (readable)1 (accessed)0
    db 11001111b;(granularity)1 (32-bit)1 (64-bit)0 (AVL)0
                ;Limit(16-19)
    db 0x0      ;Base(24-31)

gdt_data_seg:
    dw 0xffff   ;Limit(0-15)
    dw 0x0      ;Base (0-15)
    db 0x0      ;Base (16-23)
    db 10010010b;(present) 1 (privilege)00 (dscpt type) 1
                ;(code) 0 (expand down)0 (writable)1 (accessed)0
    db 11001111b;(granularity)1 (32-bit)1 (64-bit)0 (AVL)0
                ;Limit(16-19)
    db 0x0      ;Base(24-31)

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code_seg - gdt_start
DATA_SEG equ gdt_data_seg - gdt_start

[bits 16]
switch_to_pm:
    cli
seta_20_1:
    in al, 0x64
    test al, 0x2
    jnz seta_20_1
    mov al, 0xd1
    out 0x64, al
seta_20_2:
    in al, 0x64
    test al, 0x2
    jnz seta_20_2
    mov al, 0xdf
    out 0x60, al


    lgdt [gdt_descriptor]

    mov eax, cr0
    or  eax, 0x1
    mov cr0, eax

    jmp CODE_SEG:init_pm

[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000
    mov esp, ebp

PM_START:
    mov bx, MSG_PM
    call print_str_pm
    call STAGE_2_OFFSET
    mov bx, MSG_LOAD_ERR
    call print_str_pm
    jmp $

BOOT_DRIVE: db 0
MSG_Hello: db 'Booting in Real Mode', 0x0a, 0x0d, 0
MSG_PM:    db 'Landed in Protected Mode', 0
MSG_DISK_ERR: db 'Disk Read Error', 0x0a, 0x0d,0
MSG_LOAD_STAGE_2: db 'Loading Stage 2', 0x0a, 0x0d,0
MSG_LOAD_ERR: db 'Kernel Not found', 0
;padding and magic number
times 510 - ($-$$) db 0
dw 0xaa55
