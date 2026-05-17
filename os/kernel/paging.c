#include "paging.h"

#define PAGE_PRESENT 0x1
#define PAGE_WRITABLE 0x2
#define PAGE_USER 0x4

static unsigned int page_directory[1024] __attribute__((aligned(4096)));
static unsigned int page_table[1024] __attribute__((aligned(4096)));

extern void paging_enable(unsigned int *page_directory);

void paging_init() {
    for (int i = 0; i < 1024; i++) {
        page_table[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
    }

    page_directory[0] = ((unsigned int)page_table) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;

    for (int i = 1; i < 1024; i++) {
        page_directory[i] = 0;
    }

    paging_enable(page_directory);
}
