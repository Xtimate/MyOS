CC = gcc
AS = nasm
CFLAGS = -m32 -ffreestanding -fno-stack-protector -nostdlib -Ikernel/include -mno-sse

user/hello:
	$(CC) -m32 -nostdlib -nostartfiles -static -fno-pic -T user/user.ld -o user/hello user/hello.c

fs.tar: user/hello user/loop
	cd user && tar cf ../fs.tar hello loop

user/loop:
	$(CC) -m32 -nostdlib -nostartfiles -static -fno-pic -T user/user.ld -o user/loop user/loop.c

kernel/kernel.bin: fs.tar kernel/arch/boot.asm kernel/gdt.c kernel/arch/gdt.asm kernel/idt.c kernel/arch/idt.asm kernel/isr.c kernel/arch/isr.asm kernel/irq.c kernel/arch/irq.asm kernel/keyboard.c kernel/vga.c kernel/shell.c kernel/kmalloc.c kernel/timer.c kernel/kstring.c kernel/paging.c kernel/arch/usermode.asm kernel/syscall.c kernel/arch/syscall.asm kernel/elf.c kernel/fs.c kernel/exec.c kernel/process.c kernel/scheduler.c kernel/arch/fs_bin.asm kernel/arch/switch.asm kernel/kernel.c
	$(AS) -f elf32 kernel/arch/boot.asm -o kernel/arch/boot.o
	$(CC) $(CFLAGS) -c kernel/gdt.c -o kernel/gdt.o
	$(AS) -f elf32 kernel/arch/gdt.asm -o kernel/arch/gdt_flush.o
	$(CC) $(CFLAGS) -c kernel/idt.c -o kernel/idt.o
	$(AS) -f elf32 kernel/arch/idt.asm -o kernel/arch/idt_load.o
	$(CC) $(CFLAGS) -c kernel/isr.c -o kernel/isr.o
	$(AS) -f elf32 kernel/arch/isr.asm -o kernel/arch/isr.o
	$(CC) $(CFLAGS) -c kernel/irq.c -o kernel/irq.o
	$(AS) -f elf32 kernel/arch/irq.asm -o kernel/arch/irq.o
	$(CC) $(CFLAGS) -c kernel/paging.c -o kernel/paging.o
	$(AS) -f elf32 kernel/arch/usermode.asm -o kernel/arch/usermode.o
	$(CC) $(CFLAGS) -c kernel/syscall.c -o kernel/syscall.o
	$(AS) -f elf32 kernel/arch/syscall.asm -o kernel/arch/syscall.o
	$(CC) $(CFLAGS) -c kernel/keyboard.c -o kernel/keyboard.o
	$(CC) $(CFLAGS) -c kernel/vga.c -o kernel/vga.o
	$(CC) $(CFLAGS) -c kernel/shell.c -o kernel/shell.o
	$(CC) $(CFLAGS) -c kernel/timer.c -o kernel/timer.o
	$(CC) $(CFLAGS) -c kernel/kstring.c -o kernel/kstring.o
	$(CC) $(CFLAGS) -c kernel/kmalloc.c -o kernel/kmalloc.o
	$(CC) $(CFLAGS) -c kernel/elf.c -o kernel/elf.o
	$(CC) $(CFLAGS) -c kernel/fs.c -o kernel/fs.o
	$(CC) $(CFLAGS) -c kernel/exec.c -o kernel/exec.o
	$(CC) $(CFLAGS) -c kernel/process.c -o kernel/process.o
	$(CC) $(CFLAGS) -c kernel/scheduler.c -o kernel/scheduler.o
	$(AS) -f elf32 kernel/arch/fs_bin.asm -o kernel/arch/fs_bin.o
	$(AS) -f elf32 kernel/arch/switch.asm -o kernel/arch/switch.o
	$(CC) $(CFLAGS) -c kernel/kernel.c -o kernel/kernel.o
	ld -m elf_i386 -T kernel/linker.ld kernel/arch/boot.o kernel/arch/gdt_flush.o kernel/gdt.o kernel/arch/idt_load.o kernel/idt.o kernel/arch/isr.o kernel/isr.o kernel/arch/irq.o kernel/irq.o kernel/keyboard.o kernel/vga.o kernel/shell.o kernel/kernel.o kernel/kmalloc.o kernel/timer.o kernel/kstring.o kernel/paging.o kernel/arch/usermode.o kernel/syscall.o kernel/arch/syscall.o kernel/elf.o kernel/fs.o kernel/exec.o kernel/process.o kernel/scheduler.o kernel/arch/fs_bin.o kernel/arch/switch.o -o kernel/kernel.bin
iso: kernel/kernel.bin
	mkdir -p iso/boot/grub
	cp kernel/kernel.bin iso/boot/kernel.bin
	cp boot/grub/grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso iso
run: iso
	qemu-system-x86_64 -cdrom myos.iso -display gtk -no-reboot -no-shutdown
clean:
	rm -rf iso kernel/*.bin kernel/*.o kernel/arch/*.o myos.iso user/hello user/loop fs.tar
