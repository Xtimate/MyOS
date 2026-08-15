CC = gcc
UCC = i686-elf-gcc
AS = nasm
CFLAGS = -m32 -ffreestanding -fno-stack-protector -nostdlib -Ikernel/include -mno-sse $(EXTRA_CFLAGS)

NEWLIB = $(HOME)/opt/cross/i686-elf
UCFLAGS = -m32 -nostartfiles -static -fno-pic -fno-stack-protector \
          -I$(NEWLIB)/include -L$(NEWLIB)/lib

USER_PROGS = user/hello user/loop user/inp user/filetest user/brktest user/hello2 user/complex user/args user/sleeptest

user/syscalls.o:
	$(UCC) $(UCFLAGS) -c user/syscalls.c -o user/syscalls.o

user/brktest: user/syscalls.o
	$(UCC) $(UCFLAGS) -T user/user.ld -o user/brktest user/brktest.c user/syscalls.o $(NEWLIB)/lib/libc.a

user/inp: user/syscalls.o
	$(UCC) $(UCFLAGS) -T user/user.ld -o user/inp user/inp.c user/syscalls.o $(NEWLIB)/lib/libc.a

user/filetest: user/syscalls.o
	$(UCC) $(UCFLAGS) -T user/user.ld -o user/filetest user/filetest.c user/syscalls.o $(NEWLIB)/lib/libc.a

user/%: user/%.c user/syscalls.o user/crt0.o
	$(UCC) $(UCFLAGS) -T user/user.ld -o $@ $< user/crt0.o user/syscalls.o $(NEWLIB)/lib/libc.a

fs.tar: $(USER_PROGS)
	cd user && tar cf ../fs.tar hello loop inp filetest hello.txt brktest hello2 complex args micropython sleeptest

kernel/kernel.bin: fs.tar kernel/arch/boot.asm kernel/gdt.c kernel/arch/gdt.asm kernel/idt.c kernel/arch/idt.asm kernel/isr.c kernel/arch/isr.asm kernel/irq.c kernel/arch/irq.asm kernel/keyboard.c kernel/mouse.c kernel/cursor.c kernel/vga.c kernel/shell.c kernel/kmalloc.c kernel/timer.c kernel/kstring.c kernel/paging.c kernel/arch/usermode.asm kernel/syscall.c kernel/arch/syscall.asm kernel/elf.c kernel/fs.c kernel/exec.c kernel/process.c kernel/scheduler.c kernel/arch/fs_bin.asm kernel/arch/switch.asm kernel/input.c kernel/pci.c kernel/framebuffer.c kernel/drivers/driver.c kernel/drivers/rtl8139.c kernel/net/arp.c kernel/net/net.c kernel/net/ipv4.c kernel/net/icmp.c kernel/net/udp.c kernel/net/dhcp.c kernel/kernel.c
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
	$(CC) $(CFLAGS) -c kernel/mouse.c -o kernel/mouse.o
	$(CC) $(CFLAGS) -c kernel/cursor.c -o kernel/cursor.o
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
	$(CC) $(CFLAGS) -c kernel/input.c -o kernel/input.o
	$(CC) $(CFLAGS) -c kernel/framebuffer.c -o kernel/framebuffer.o
	$(CC) $(CFLAGS) -c kernel/pci.c -o kernel/pci.o
	$(CC) $(CFLAGS) -c kernel/drivers/driver.c -o kernel/drivers/driver.o
	$(CC) $(CFLAGS) -c kernel/drivers/rtl8139.c -o kernel/drivers/rtl8139.o
	$(CC) $(CFLAGS) -c kernel/net/arp.c -o kernel/net/arp.o
	$(CC) $(CFLAGS) -c kernel/net/net.c -o kernel/net/net.o
	$(CC) $(CFLAGS) -c kernel/net/ipv4.c -o kernel/net/ipv4.o
	$(CC) $(CFLAGS) -c kernel/net/icmp.c -o kernel/net/icmp.o
	$(CC) $(CFLAGS) -c kernel/net/udp.c -o kernel/net/udp.o
	$(CC) $(CFLAGS) -c kernel/net/dhcp.c -o kernel/net/dhcp.o
	$(CC) $(CFLAGS) -c kernel/kernel.c -o kernel/kernel.o
	ld -m elf_i386 -T kernel/linker.ld kernel/arch/boot.o kernel/arch/gdt_flush.o kernel/gdt.o kernel/arch/idt_load.o kernel/idt.o kernel/arch/isr.o kernel/isr.o kernel/arch/irq.o kernel/irq.o kernel/keyboard.o kernel/cursor.o kernel/mouse.o kernel/vga.o kernel/shell.o kernel/kernel.o kernel/kmalloc.o kernel/timer.o kernel/kstring.o kernel/paging.o kernel/arch/usermode.o kernel/syscall.o kernel/arch/syscall.o kernel/elf.o kernel/fs.o kernel/exec.o kernel/process.o kernel/scheduler.o kernel/arch/fs_bin.o kernel/arch/switch.o kernel/input.o kernel/pci.o kernel/framebuffer.o kernel/drivers/driver.o kernel/drivers/rtl8139.o kernel/net/arp.o kernel/net/net.o kernel/net/ipv4.o kernel/net/icmp.o kernel/net/udp.o kernel/net/dhcp.o -o kernel/kernel.bin

iso: kernel/kernel.bin
	mkdir -p iso/boot/limine
	mkdir -p iso/EFI/BOOT
	cp kernel/kernel.bin iso/boot/kernel.bin
	cp boot/limine/limine.conf iso/boot/limine/limine.conf
	cp /usr/share/limine/limine-bios.sys iso/boot/limine/
	cp /usr/share/limine/limine-bios-cd.bin iso/boot/limine/
	cp /usr/share/limine/limine-uefi-cd.bin iso/boot/limine/
	cp /usr/share/limine/BOOTX64.EFI iso/EFI/BOOT/
	xorriso -as mkisofs -R -J -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso -o myos.iso
	limine bios-install myos.iso

run: iso
	qemu-system-x86_64 -cdrom myos.iso -display gtk -no-reboot -no-shutdown -vga std -netdev tap,id=net0,ifname=tap0,script=no,downscript=no -device rtl8139,netdev=net0 -serial mon:stdio -d int,cpu_reset -D /tmp/qemu.log

clean:
	rm -rf iso kernel/*.bin kernel/*.o kernel/arch/*.o kernel/drivers/*.o kernel/net/*.o myos.iso $(USER_PROGS) user/syscalls.o fs.tar
