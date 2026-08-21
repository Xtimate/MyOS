# myOS

A hobby x86 32-bit operating system I built from scratch in C and NASM. No existing kernel or libc.

## What it does

myOS boots via [Limine](https://github.com/limine-bootloader/limine)(multiboot2) and I wrote prett much everything myself with the help of the OSDev wiki and Claude AI:

- **Memory management**: PAE (Physical Address Extension) paging, which I had to add because real UEFI hardware kept putting the framebuffer above 4G and my old 32-bit paging couldn't reach it.
- **Interrupts and scheduling**: custom GDT/IDT/IRQ setup, a process scheduler, and a syscall interface (`int 0x80`) so user programs can actually do stuff.
- **Drivers**: PCI enumeration, an RTL8139 network card driver, PS/2 keyboard and mouse drivers.
- **Networking**: I wrote a whole mini network stack from scratch, ARP, IPv4, ICMP, UDP and DHCP.
- **Graphics**: A linear framebuffer driver (1920x1080x32) with a bitmap font renderer, a terminal, and a mouse cursor I drew and coded myself.
- **Userspace**: An ELF loader, exec, a simple tar based filesystem and some user programs.
- **Shell**: A basic interactive shell, supports Ctrl+C and Ctrl+Z for killing/backgrounding processes.

## Building

You need an `i686-elf` cross compiler toolchain, `nasm`, `xorriso` and `limine` installed.

```sh
make iso
```

This spits out `myos.iso`, a hybrid BIOS/UEFI bootable image.

## Running

**QEMU (what I use for testing):**

```sh
qemu-system-x86_64 -bios /usr/share/edk2/x64/OVFM.4m.fd -cdrom myos.iso -vga std
```

**Real hardware:** flash `myos.iso` onto a USB with something like [Ventoy](https://www.ventoy.net/) and boot from it. You need PAE support for UEFI boot to actually work right, and it's on by default so you should be fine.

## Project structure

```
kernel/
├── arch/       # boot code and low level asm (GDT, IDT, IRQ, syscalls, context switching)
├── drivers/    # PCI, RTL8139
├── net/        # ARP, IPv4, ICMP, UDP, DHCP
├── include/    # headers
└── *.c         # core kernel stuff: paging, process management, scheduler, filesystem, framebuffer, shell
user/           # userspace programs and the tiny C runtime they link against
boot/limine/    # bootloader config
```

## Why PAE?

So basically, on real UEFI hardware the framebuffer's physical address can end up above 4GB, and plain 32-bit paging literally cannot point at that. I had to rewrite mywhole paging system just to map that one region correctly, while keeping the rest of the kernel running on 32-bit virtual addresses like normal. This took me forever to debug lol.

## License

MIT, see [LICENSE](LICENSE)
