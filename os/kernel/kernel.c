#include "include/gdt.h"
#include "include/idt.h"
#include "include/process.h"
#include "include/isr.h"
#include "include/irq.h"
#include "include/keyboard.h"
#include "include/vga.h"
#include "include/shell.h"
#include "include/kmalloc.h"
#include "include/timer.h"
#include "include/paging.h"
#include "include/usermode.h"
#include "include/syscall.h"
#include "include/elf.h"
#include "include/fs.h"
#include "include/process.h"
#include "include/input.h"
#include "include/framebuffer.h"

typedef struct {
    unsigned int flags;
    unsigned int mem_lower;
    unsigned int mem_upper;
    unsigned int boot_device;
    unsigned int cmdline;
    unsigned int mods_count;
    unsigned int mods_addr;
    unsigned int syms[4];
    unsigned int mmap_length;
    unsigned int mmap_addr;
    unsigned int drives_length;
    unsigned int drive_addr;
    unsigned int config_table;
    unsigned int boot_loader_name;
    unsigned int apm_table;
    unsigned int vbe_control_info;
    unsigned int vbe_mode_info;
    unsigned short vbe_mode;
    unsigned short vbe_interface_seg;
    unsigned short vbe_interface_off;
    unsigned short vbe_interface_len;
    unsigned long long framebuffer_addr;
    unsigned int framebuffer_pitch;
    unsigned int framebuffer_width;
    unsigned int framebuffer_height;
    unsigned char framebuffer_bpp;
    unsigned char framebuffer_type;
} multiboot_info_t;

extern char fs_archive_start;
extern char fs_archive_end;

void shell_thread() {
    shell_init();
    while (1) { __asm__ volatile ("hlt"); }
}

void kernel_main(multiboot_info_t *mbi) {
    vga_init();
    vga_print("flags: ");
    vga_print_num(mbi->flags);
    vga_print("\n");
    gdt_install();
    vga_print("gdt ok\n");
    idt_install();
    vga_print("idt ok\n");
    isr_install();
    vga_print("isr ok\n");
    syscall_install();
    vga_print("syscall ok\n");
    irq_install();
    vga_print("irq ok\n");
    keyboard_install();
    vga_print("keyboard ok\n");
    timer_install(100);
    vga_print("timer ok\n");
    kmalloc_init(0xC0500000, 0xC0600000);
    vga_print("kmalloc ok\n");
    paging_init();
    vga_print("paging ok\n");
    process_init();
    vga_print("process ok\n");
    input_init();
    vga_print("input ok\n");

    if (mbi->flags & (1 << 12)) {
        fb_init(
            (unsigned char *)(unsigned int)mbi->framebuffer_addr,
            mbi->framebuffer_width,
            mbi->framebuffer_height,
            mbi->framebuffer_pitch,
            mbi->framebuffer_bpp
        );
        fb_clear(0x00FF0000);
        vga_print("fb ok\n");
    } else {
        vga_print("no fb\n");
    }

    __asm__ volatile ("sti");

    fs_init(&fs_archive_start, &fs_archive_end - &fs_archive_start);
    vga_print("fs ok\n");

    process_t *shell = process_create_kernel(shell_thread);
    current_process = shell;
    shell->state = PROCESS_STATE_RUNNING;

    shell_thread();
}
