ENTRY(start)
KERNEL_VIRTUAL_BASE = 0xC0000000;

PHDRS {
    boot   PT_LOAD FLAGS(5);
    kernel PT_LOAD FLAGS(7);
    bss    PT_LOAD FLAGS(6);
}

SECTIONS {
    . = 0x100000;

    .multiboot2 : { *(.multiboot2) } :boot
    .multiboot  : { *(.multiboot) }  :boot
    .boot       : { *(.boot) }       :boot

    . = 0xC0100000 + (. - 0x100000);

    .text : AT(ADDR(.text) - KERNEL_VIRTUAL_BASE) {
        *(.text)
        *(.text.*)
    } :kernel
    .data : AT(ADDR(.data) - KERNEL_VIRTUAL_BASE) {
        *(.data)
        *(.data.*)
        *(.rodata)
        *(.rodata.*)
    } :kernel
    .bss : AT(ADDR(.bss) - KERNEL_VIRTUAL_BASE) {
        bss_start = .;
        *(.bss)
        *(.bss.*)
        bss_end = .;
    } :bss

    /DISCARD/ : {
        *(.eh_frame)
        *(.comment)
    }
}
