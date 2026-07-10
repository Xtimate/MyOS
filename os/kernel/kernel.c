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
#include "include/syscall.h"
#include "include/fs.h"
#include "include/input.h"
#include "include/framebuffer.h"
#include "include/pci.h"
#include "drivers/driver.h"
#include "net/arp.h"
#include "drivers/rtl8139.h"

typedef struct {
    unsigned int type;
    unsigned int size;
} mb2_tag_t;

typedef struct {
    unsigned int type;
    unsigned int size;
    unsigned long long addr;
    unsigned int pitch;
    unsigned int width;
    unsigned int height;
    unsigned char bpp;
    unsigned char fb_type;
    unsigned short reserved;
} mb2_tag_framebuffer_t;

extern char fs_archive_start;
extern char fs_archive_end;

void shell_thread() {
    shell_init();
    while (1) { __asm__ volatile ("hlt"); }
}

static void serial_print(const char *s) {
    while (*s) {
        __asm__ volatile ("outb %0, $0x3F8" : : "a"(*s));
        s++;
    }
}

static void serial_hex(unsigned int n) {
    char buf[9]; buf[8] = 0;
    for (int i = 7; i >= 0; i--) {
        int x = n & 0xF;
        buf[i] = x < 10 ? '0'+x : 'a'+x-10;
        n >>= 4;
    }
    serial_print(buf);
}

void kernel_main(unsigned int mb2_magic, unsigned int mb2_addr) {
    vga_init();

    gdt_install();
    idt_install();
    isr_install();
    syscall_install();
    irq_install();
    keyboard_install();
    timer_install(100);
    kmalloc_init(0xC0500000, 0xC0600000);
    paging_init();
    process_init();
    input_init();
    pci_enumerate();

    vga_print("magic: ");
    vga_print_num(mb2_magic);
    vga_print("\n");

    // parse multiboot2 tags
    unsigned int addr = mb2_addr + 8; // skip total_size and reserved
    mb2_tag_framebuffer_t *fb_tag = 0;

    while (1) {
        mb2_tag_t *tag = (mb2_tag_t *)addr;
        if (tag->type == 0) break; // end tag

        if (tag->type == 8) { // framebuffer tag
            fb_tag = (mb2_tag_framebuffer_t *)tag;
        }

        addr = (addr + tag->size + 7) & ~7; // align to 8 bytes
    }

    if (fb_tag) {
        unsigned int fb_phys = (unsigned int)fb_tag->addr;
        vga_print("fb phys: "); vga_print_num(fb_phys); vga_print("\n");
        vga_print("w: "); vga_print_num(fb_tag->width); vga_print("\n");
        vga_print("h: "); vga_print_num(fb_tag->height); vga_print("\n");
        vga_print("pitch: "); vga_print_num(fb_tag->pitch); vga_print("\n");
        vga_print("bpp: "); vga_print_num(fb_tag->bpp); vga_print("\n");

        unsigned int fb_size = fb_tag->pitch * fb_tag->height;

        // init serial port 0x3F8
        __asm__ volatile ("outb %0, $0x3F9" : : "a"((unsigned char)0x00)); // disable interrupts
        __asm__ volatile ("outb %0, $0x3FB" : : "a"((unsigned char)0x80)); // enable DLAB
        __asm__ volatile ("outb %0, $0x3F8" : : "a"((unsigned char)0x03)); // baud lo
        __asm__ volatile ("outb %0, $0x3F9" : : "a"((unsigned char)0x00)); // baud hi
        __asm__ volatile ("outb %0, $0x3FB" : : "a"((unsigned char)0x03)); // 8N1
        __asm__ volatile ("outb %0, $0x3FC" : : "a"((unsigned char)0x03)); // RTS/DSR

        serial_print("pd_phys: ");
        serial_hex((unsigned int)page_directory - 0xC0000000);
        serial_print("\n");
        paging_map_framebuffer(fb_phys, fb_size);
        serial_print("pd[1008]: ");
        serial_hex(paging_get_pd_entry(1008));
        serial_print("\n");

        paging_map_framebuffer(fb_phys, fb_size);

        fb_init(
            (unsigned char *)paging_get_fb_virt(),
            fb_tag->width,
            fb_tag->height,
            fb_tag->pitch,
            fb_tag->bpp
        );
        fb_clear(0x00000000);
        fb_terminal_init();
        drivers_init_all();

        unsigned char my_ip[4] = {10, 0, 2, 15};
        unsigned char mac[6];
        rtl8139_get_mac(mac);
        arp_init(mac, my_ip);

        vga_print("fb ok\n");
    } else {
        vga_print("no fb tag\n");
    }

    __asm__ volatile ("sti");

    fs_init(&fs_archive_start, &fs_archive_end - &fs_archive_start);

    process_t *shell = process_create_kernel(shell_thread);
    current_process = shell;
    shell->state = PROCESS_STATE_RUNNING;

    shell_thread();
}
