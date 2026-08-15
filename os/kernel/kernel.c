#include "include/cursor.h"
#include "include/gdt.h"
#include "include/idt.h"
#include "include/mouse.h"
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
#include "net/ipv4.h"
#include "net/udp.h"
#include "net/dhcp.h"

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
    while (1) {
        net_poll();
        cursor_update();
        __asm__ volatile ("hlt");
    }
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

// --- capslock LED blink checkpoints (real-hardware debug aid, no serial/display needed) ---
// Build with `-DQEMU_BUILD` to make these instant no-ops for fast QEMU iteration;
// omit the flag (plain `make iso`) for real-hardware test builds where you need the LED signal.
#ifdef QEMU_BUILD

static void blink_capslock(int times) {
    (void)times; // no-op under QEMU - use serial_print/serial_hex instead
}

#else

static inline unsigned char kbd_inb() {
    unsigned char v;
    __asm__ volatile ("inb $0x64, %0" : "=a"(v));
    return v;
}

static inline void kbd_wait() {
    while (kbd_inb() & 0x02);
}

static void delay_long(unsigned int outer) {
    for (unsigned int o = 0; o < outer; o++) {
        for (volatile unsigned int i = 0; i < 2000000; i++);
    }
}

static void blink_capslock(int times) {
    // long pause before the group starts, so each checkpoint is clearly separated
    delay_long(400); // ~ a few seconds

    for (int b = 0; b < times; b++) {
        kbd_wait();
        __asm__ volatile ("outb %0, $0x60" : : "a"((unsigned char)0xED)); // "Set LEDs" command
        kbd_wait();
        __asm__ volatile ("outb %0, $0x60" : : "a"((unsigned char)0x04)); // Caps Lock bit on
        delay_long(150); // ON duration - clearly visible

        kbd_wait();
        __asm__ volatile ("outb %0, $0x60" : : "a"((unsigned char)0xED));
        kbd_wait();
        __asm__ volatile ("outb %0, $0x60" : : "a"((unsigned char)0x00)); // all LEDs off
        delay_long(150); // OFF duration - clearly visible
    }
    // pause after the group so the next checkpoint's blinks aren't confused with this one
    delay_long(200);
}

#endif
// --- end blink checkpoints ---

void kernel_main(unsigned int mb2_magic, unsigned int mb2_addr) {
    // absolute first thing: init serial and print, before ANYTHING else runs
    __asm__ volatile ("outb %0, $0x3F9" : : "a"((unsigned char)0x00));
    __asm__ volatile ("outb %0, $0x3FB" : : "a"((unsigned char)0x80));
    __asm__ volatile ("outb %0, $0x3F8" : : "a"((unsigned char)0x03));
    __asm__ volatile ("outb %0, $0x3F9" : : "a"((unsigned char)0x00));
    __asm__ volatile ("outb %0, $0x3FB" : : "a"((unsigned char)0x03));
    __asm__ volatile ("outb %0, $0x3FC" : : "a"((unsigned char)0x03));
    serial_print("=== kernel_main entered ===\n");

    vga_init();

    gdt_install();
    idt_install();
    isr_install();
    syscall_install();
    irq_install();
    keyboard_install();
    mouse_install();
    cursor_init();
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
        blink_capslock(1); // checkpoint 1: framebuffer tag found, entering fb setup

        unsigned long long fb_phys = fb_tag->addr;
        vga_print("fb phys: "); vga_print_num((unsigned int)(fb_phys & 0xFFFFFFFF)); vga_print("\n");
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

        blink_capslock(2); // checkpoint 2: serial port initialized

        serial_print("pdpt_phys: ");
        serial_hex(paging_get_pdpt_phys());
        serial_print("\n");

        paging_map_framebuffer(fb_phys, fb_size);

        blink_capslock(3); // checkpoint 3: paging_map_framebuffer returned

        serial_print("pd[1008]: ");
        serial_hex(paging_get_pd_entry(1008));
        serial_print("\n");

        unsigned int *fb_test = (unsigned int *)paging_get_fb_virt();

        blink_capslock(4); // checkpoint 4: got fb virtual pointer

        for (int i = 0; i < 1000; i++) {
            fb_test[i] = 0xFFFFFFFF; // white
        }

        blink_capslock(5); // checkpoint 5: raw pixel write loop completed

        fb_init(
            (unsigned char *)paging_get_fb_virt(),
            fb_tag->width,
            fb_tag->height,
            fb_tag->pitch,
            fb_tag->bpp
        );

        blink_capslock(6); // checkpoint 6: fb_init returned

        serial_print("fb_virt (expected): ");
        serial_hex(paging_get_fb_virt());
        serial_print("\n");

        fb_clear(0x00000000);

        blink_capslock(7); // checkpoint 7: fb_clear returned

        fb_terminal_init();
        drivers_init_all();

        unsigned char my_ip[4] = {192, 168, 100, 2};
        unsigned char mac[6];
        rtl8139_get_mac(mac);
        arp_init(mac, my_ip);
        ipv4_init(my_ip);
        udp_init();
        udp_set_ip(my_ip);
        dhcp_init(mac);

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
