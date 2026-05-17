ENTRY(_start)

SECTIONS {
    . = 0x200000;

    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .got.plt : { *(.got.plt) }
    .bss : { *(.bss) }

    /DISCARD/ : {
        *(.eh_frame)
        *(.note.gnu.build-id)
        *(.note.gnu.property)
        *(.comment)
    }
}
