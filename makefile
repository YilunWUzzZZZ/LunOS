CFLAGS = -ffreestanding -fno-pie -fno-asynchronous-unwind-tables -m32
LDFLAGS = -melf_i386
LDFLAGS_KERNEL = -T kernel/link.ld -melf_i386
CSOURCES = $(wildcard kernel/*.c)
ASMSOURCES = $(wildcard kernel/*.asm)
OBJS = $(CSOURCES:.c=.o) 
OBJS += $(ASMSOURCES:.asm=.o) 

kernel.elf: $(OBJS)
	ld $(LDFLAGS_KERNEL) -o $@ $^


bochs: boot1.bin boot2.bin kernel.elf
	dd if=boot1.bin of=hd60M.img bs=512 count=1 conv=notrunc
	dd if=boot2.bin of=hd60M.img bs=512 seek=1 conv=notrunc
	dd if=kernel.elf of=hd60M.img bs=512 seek=3 conv=notrunc
	bochs -f bochscfg


boot1.bin: boot/bootloader.asm
	nasm -f bin -o $@ $<

boot2.bin: boot/boot2_entry.o boot/boot_stage_2.o  kernel/utils.o
	ld  -o $@  -Ttext 0x1000 $(LDFLAGS) --oformat binary $^


boot2_entry.o: boot/boot2_entry.asm
	nasm -f elf -o $@ $^

%.o:%.c
	gcc $(CFLAGS)  -o $@ -c  $<

%.o:%.asm
	nasm -f elf -o $@ $^

clean:
	rm kernel/*.o kernel.elf boot/*.o *.bin

qemu: disk.img
	qemu-system-i386 -hda disk.img

disk.img: boot1.bin boot2.bin kernel.elf
	dd if=/dev/zero of=$@ bs=10M count=1
	dd if=boot1.bin of=$@ bs=512 count=1 conv=notrunc
	dd if=boot2.bin of=$@ bs=512 seek=1 conv=notrunc
	dd if=kernel.elf of=$@ bs=512 seek=3 conv=notrunc