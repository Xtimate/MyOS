#include "paging.h"
#include "include/paging.h"

#define PAGE_PRESENT 0x1
#define PAGE_WRITABLE 0x2
#define PAGE_USER 0x4

#define BOOT_PAGE_TABLE1_PHYS 0x102000
#define BOOT_PAGE_TABLE2_PHYS 0x103000

#define PAGE_FRAME_START 0xC0400000
#define PAGE_FRAME_PHYS 0x00400000

static unsigned int page_directory[1024] __attribute__((aligned(4096)));
static unsigned int user_page_table[1024] __attribute__((aligned(4096)));

void paging_init() {
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0;
    }

    page_directory[768] = BOOT_PAGE_TABLE1_PHYS | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
    page_directory[769] = BOOT_PAGE_TABLE2_PHYS | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;

    for (int i = 0; i < 1024; i++) {
        user_page_table[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
    }

    unsigned int user_pt_phys = (unsigned int)user_page_table - 0xC0000000;
    page_directory[0] = user_pt_phys | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;

    unsigned int pd_phys = (unsigned int)page_directory - 0xC0000000;

    unsigned int *pt1 = (unsigned int *)0xC0102000;
    unsigned int *pt2 = (unsigned int *)0xC0103000;
    for (int i = 0; i < 1024; i++) {
        if (pt1[i] & PAGE_PRESENT) pt1[i] |= PAGE_USER;
        if (pt2[i] & PAGE_PRESENT) pt2[i] |= PAGE_USER;

    }

    __asm__ volatile ("mov %0, %%cr3" : : "r"(pd_phys));
}

static unsigned int frame_bump = PAGE_FRAME_START;

unsigned int paging_alloc_frame() {
    unsigned int virt = frame_bump;
    frame_bump += 4096;
    return virt;
}

unsigned int *paging_create_directory() {
    unsigned int *dir = (unsigned int *)paging_alloc_frame();
    unsigned int dir_phys = (unsigned int)dir - 0xC0000000;

    for (int i = 0; i < 1024; i++)
        dir[i] = 0;

    dir[768] = BOOT_PAGE_TABLE1_PHYS | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
    dir[769] = BOOT_PAGE_TABLE2_PHYS | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;

    unsigned int *upt = (unsigned int *)paging_alloc_frame();
    unsigned int upt_phys = (unsigned int)upt - 0xC0000000;

    for (int i = 0; i < 1024; i++)
        upt[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;

    dir[0] = upt_phys | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;

    return (unsigned int *)dir_phys;
}

void paging_switch(unsigned int pd_phys) {
    __asm__ volatile ("mov %0, %%cr3" : : "r"(pd_phys));
}
