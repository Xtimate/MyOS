# myOS

A hobby x86 32-bit operating system I built from scratch in C and NASM. No existing kernel or libc.

## What it does

myOS boots via [Limine](https://github.com/limine-bootloader/limine)(multiboot2) and I wrote pretty much everything myself with the help of the OSDev wiki and Claude AI:

- **Memory management**: PAE (Physical Address Extension) paging, which I had to add because real UEFI hardware kept putting the framebuffer above 4G and my old 32-bit paging couldn't reach it.
- **Interrupts and scheduling**: custom GDT/IDT/IRQ setup, a process scheduler, and a syscall interface (`int 0x80`) so user programs can actually do stuff.
- **Drivers**: PCI enumeration, an RTL8139 network card driver, PS/2 keyboard and mouse drivers.
- **Networking**: I wrote a whole mini network stack from scratch, ARP, IPv4, ICMP, UDP and DHCP.
- **Graphics**: A linear framebuffer driver (1920x1080x32) with a bitmap font renderer, a terminal, and a mouse cursor I drew and coded myself.
- **Userspace**: An ELF loader, exec, a simple tar based filesystem and some user programs.
- **Shell**: A basic interactive shell, supports Ctrl+C and Ctrl+Z for killing/backgrounding processes.

## Building

### 1. Build the cross-compiler toolchain

You need an `i686-elf` GCC cross-compiler and `newlib` built for that target. This only needs to be done once

```sh
# install build dependencies (Arch example, adjust for your distro)
sudo pacman -S base-devel gmp mpfr libmpc

mkdir -p ~/src ~/opt/cross
cd ~/src

# --- build binutils ---
curl -O https://ftp.gnu.org/gnu/binutils/binutils-2.42.tar.gz
tar xf binutils-2.42.tar.gz
mkdir build-binutils && cd build-binutils
../binutils-2.42/configure --target=i686-elf --prefix="$HOME/opt/cross" --with-sysroot --disable-nls --disable-werror
make -j$(nproc)
make install
cd ..

# --- build GCC (use 15.2.0 specifically - older 13.x has known build
#     issues against newer host compilers) ---
curl -O https://ftp.gnu.org/gnu/gcc/gcc-15.2.0/gcc-15.2.0.tar.gz
tar xf gcc-15.2.0.tar.gz
mkdir build-gcc && cd build-gcc
../gcc-15.2.0/configure --target=i686-elf --prefix="$HOME/opt/cross" \
    --disable-nls --enable-languages=c --without-headers --disable-lto \
    CXXFLAGS="-fno-char8_t"
make -j$(nproc) all-gcc all-target-libgcc
make install-gcc install-target-libgcc
cd ..

export PATH="$HOME/opt/cross/bin:$PATH"   # add this to your shell config too
```

### 2. Build newlib for the target

```sh
curl -O ftp://sourceware.org/pub/newlib/newlib-4.4.0.20231231.tar.gz
tar xf newlib-4.4.0.20231231.tar.gz
mkdir build-newlib && cd build-newlib
../newlib-4.4.0.20231231/configure --target=i686-elf --prefix="$HOME/opt/cross"
make -j$(nproc)
make install
```

If `make install` fails with `cannot stat 'libcygmon.a'`, that's a known dead libgloss board-support target that doesn't apply here — just create a dummy placeholder and re-run `make install`, no rebuild needed:

```sh
touch i686-elf/libgloss/i386/libcygmon.a
make install
```

### 3. Install the remaining build tools
 
```sh
sudo pacman -S nasm xorriso limine edk2-ovmf qemu-system-x86 qemu-ui-gtk qemu-ui-sdl
```
 
(`edk2-ovmf` provides the UEFI firmware image QEMU needs to test UEFI boot; `qemu-ui-gtk`/`qemu-ui-sdl` are separate packages on Arch and QEMU won't have a display without at least one of them installed)

### 4. Build the ISO

```sh
make iso
```

This spits out `myos.iso`, a hybrid BIOS/UEFI bootable image.

## Running

**QEMU (what I use for testing):**

```sh
qemu-system-x86_64 -bios /usr/share/edk2/x64/OVFM.4m.fd -cdrom myos.iso -vga std
```

A quick rundown of the less obvious flags:
- `-bios .../OVFM.4m.fd` UEFI firmware, needed since the OS boots via UEFI/GOP, not legacy BIOS
- `-netdev tap,...` / `-device rtl8139` networking, needs a `tap0` interface set up on the host first (see `make netsetup` in the Makefile, or set one up manually with `ip tuntap`)
- `-no-reboot -no-shutdown` if the kernel crashes, QEMU freezes instead of silently resetting, so you can actually see what happened
- `-serial mon:stdio` routes the kernel's serial debug output to your terminal
- `-d int,cpu_reset -D /tmp/qemu.log` logs CPU exceptions/resets to a file, useful for debugging faults that don't print anything on screen

If you don't need networking, you can drop the `-netdev`/`-device` flags and just run:

```sh
qemu-system-x86_64 -bios /usr/share/edk2/x64/OVMF.4m.fd -cdrom myos.iso -vga std
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

## Known limitations

- **Real hardware UEFI boot is not fully stable yet.** It boots and runs, but there's an intermittent crash I haven't fully root-caused yet, triggered by running certain user programs. It doesn't hapen in QEMU, only on real hardware, which makes it a lot harder to debug. Still actively working on this.
- **There's leftover debug instrumentation for real hardware builds.** Since real hardware doesn't have working serial output for me to debug with, I added a fallback: the kernel blinks the Caps Lock LED at specific checkpoints during boot as a debug signal (see `kernel/kernel.c`, gated behind a `QEMU_BUILD` compile flag). This intentionally slows down boot on real hardware with extra delays. It's meant to be removed once the hardware crash mentioned above is resolved, but it's still in there for now.

## Why PAE?

So basically, on real UEFI hardware the framebuffer's physical address can end up above 4GB, and plain 32-bit paging literally cannot point at that. I had to rewrite mywhole paging system just to map that one region correctly, while keeping the rest of the kernel running on 32-bit virtual addresses like normal. This took me forever to debug lol.

## License

MIT, see [LICENSE](LICENSE)
