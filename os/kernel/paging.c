#include "paging.h"

#define PAGE_PRESENT 0x1
#define PAGE_WRITABLE 0x2
#define PAGE_USER 0x4

#define BOOT_PAGE_TABLE1_PHYS 0x102000
#define BOOT_PAGE_TABLE2_PHYS 0x103000

static unsigned int page_directory[1024] __attribute__((aligned(4096)));
static unsigned int user_page_table[1024] __attribute__((aligned(4096)));

void paging_init() {
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0;
    }

    page_directory[768] = BOOT_PAGE_TABLE1_PHYS | PAGE_PRESENT | PAGE_WRITABLE;
    page_directory[769] = BOOT_PAGE_TABLE2_PHYS | PAGE_PRESENT | PAGE_WRITABLE;

    for (int i = 0; i < 1024; i++) {
        user_page_table[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
    }

    unsigned int user_pt_phys = (unsigned int)user_page_table - 0xC0000000;
    page_directory[0] = user_pt_phys | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;

    unsigned int pd_phys = (unsigned int)page_directory - 0xC0000000;

    __asm__ volatile ("mov %0, %%cr3" : : "r"(pd_phys));
}
