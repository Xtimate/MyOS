#include "include/paging.h"

typedef unsigned long long u64;

#define PAGE_PRESENT  0x1ULL
#define PAGE_WRITABLE 0x2ULL
#define PAGE_USER     0x4ULL

#define KERNEL_VIRTUAL_BASE 0xC0000000u
#define PAGE_FRAME_START 0xC0700000u   // bump allocator start (virtual) - clear of kmalloc's 0xC0500000-0xC0600000 heap and well within the 16MB kernel-mapped range (0xC0000000-0xC1000000)
#define FB_VIRT_BASE 0xFD000000u

// --- permanent PAE structures, built by paging_init() ---
// These replace boot.asm's temporary bootstrap tables entirely; CR3 is
// re-pointed at these once paging_init() runs.
static u64 pdpt[4] __attribute__((aligned(32)));

// Kernel higher-half mapping: PDPT[3] covers virt 0xC0000000-0xFFFFFFFF.
// 8 page tables * 2MB each = 16MB of kernel space, identity-mirrored
// (phys 0x000000-0xFFFFFF <-> virt 0xC0000000-0xC0FFFFFF).
#define KERNEL_PT_COUNT 8
static u64 pd_kernel[512] __attribute__((aligned(4096)));
static u64 pt_kernel[KERNEL_PT_COUNT][512] __attribute__((aligned(4096)));

// Low identity + user-accessible mapping: PDPT[0] covers virt 0x00000000-0x3FFFFFFF.
// Only the first 2MB actually populated (mirrors old user_page_table's intent).
static u64 pd_user[512] __attribute__((aligned(4096)));
static u64 pt_user[512] __attribute__((aligned(4096)));

static u64 pd_kernel_phys_g;   // physical address of pd_kernel, saved for paging_create_directory()
static u64 pdpt_phys_g;        // physical address of the main kernel pdpt

static inline unsigned int phys_of(void *virt_in_kernel_space) {
    return (unsigned int)virt_in_kernel_space - KERNEL_VIRTUAL_BASE;
}

void paging_init() {
    for (int i = 0; i < 4; i++) pdpt[i] = 0;
    for (int i = 0; i < 512; i++) { pd_kernel[i] = 0; pd_user[i] = 0; }

    for (int i = 0; i < 512; i++) {
        pt_user[i] = ((u64)(i * 0x1000)) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
    }

    for (int t = 0; t < KERNEL_PT_COUNT; t++) {
        for (int i = 0; i < 512; i++) {
            unsigned int phys = (unsigned int)(t * 0x200000) + i * 0x1000;
            pt_kernel[t][i] = ((u64)phys) | PAGE_PRESENT | PAGE_WRITABLE;
        }
        u64 pt_phys = (u64)phys_of(pt_kernel[t]);
        pd_kernel[t] = pt_phys | PAGE_PRESENT | PAGE_WRITABLE;
    }

    u64 pt_user_phys = (u64)phys_of(pt_user);
    pd_user[0] = pt_user_phys | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;

    pd_kernel_phys_g = (u64)phys_of(pd_kernel);
    u64 pd_user_phys  = (u64)phys_of(pd_user);

    pdpt[0] = pd_user_phys   | PAGE_PRESENT;  // virt 0x00000000-0x3FFFFFFF
    pdpt[3] = pd_kernel_phys_g | PAGE_PRESENT; // virt 0xC0000000-0xFFFFFFFF

    pdpt_phys_g = (u64)phys_of(pdpt);

    unsigned int pdpt_phys32 = (unsigned int)pdpt_phys_g;
    __asm__ volatile (
        "mov %0, %%cr3\n"
        : : "r"(pdpt_phys32) : "memory"
    );
}

static unsigned int frame_bump = PAGE_FRAME_START;
unsigned int paging_alloc_frame() {
    unsigned int virt = frame_bump;
    frame_bump += 4096;
    return virt;
}

// Physical-address bump allocator for identity-ish scratch physical frames
// (kept from the original file; used wherever raw physical frames are needed
// outside the initial kernel/user mappings).
static unsigned int phys_bump = 0x01000000;
unsigned int paging_alloc_phys_frame() {
    unsigned int phys = phys_bump;
    phys_bump += 4096;
    return phys;
}

void paging_switch(unsigned int pdpt_phys) {
    __asm__ volatile ("mov %0, %%cr3" : : "r"(pdpt_phys) : "memory");
}

// Core PAE mapping primitive. phys is a full 64-bit physical address now,
// so this can map frames anywhere in physical memory, not just below 4GB.
void paging_map_page(unsigned int pdpt_phys, unsigned int virt, u64 phys) {
    u64 *pdpt_ptr = (u64 *)(pdpt_phys + KERNEL_VIRTUAL_BASE);
    unsigned int pdpt_index = (virt >> 30) & 0x3;
    unsigned int pd_index   = (virt >> 21) & 0x1FF;
    unsigned int pt_index   = (virt >> 12) & 0x1FF;

    if (!(pdpt_ptr[pdpt_index] & PAGE_PRESENT)) {
        u64 *pd = (u64 *)paging_alloc_frame();
        for (int i = 0; i < 512; i++) pd[i] = 0;
        u64 pd_phys = (u64)phys_of(pd);
        pdpt_ptr[pdpt_index] = pd_phys | PAGE_PRESENT;
    }
    u64 pd_phys_entry = pdpt_ptr[pdpt_index] & ~0xFFFULL;
    u64 *pd = (u64 *)((unsigned int)pd_phys_entry + KERNEL_VIRTUAL_BASE);

    if (!(pd[pd_index] & PAGE_PRESENT)) {
        u64 *pt = (u64 *)paging_alloc_frame();
        for (int i = 0; i < 512; i++) pt[i] = 0;
        u64 pt_phys = (u64)phys_of(pt);
        pd[pd_index] = pt_phys | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
    }
    u64 pt_phys_entry = pd[pd_index] & ~0xFFFULL;
    u64 *pt = (u64 *)((unsigned int)pt_phys_entry + KERNEL_VIRTUAL_BASE);

    pt[pt_index] = (phys & ~0xFFFULL) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
}

unsigned int paging_create_directory() {
    u64 *new_pdpt = (u64 *)paging_alloc_frame();
    for (int i = 0; i < 4; i++) new_pdpt[i] = 0;
    new_pdpt[3] = pd_kernel_phys_g | PAGE_PRESENT; // share kernel's higher-half mapping

    u64 *new_pd_user = (u64 *)paging_alloc_frame();
    for (int i = 0; i < 512; i++) new_pd_user[i] = 0;

    u64 *new_pt_user = (u64 *)paging_alloc_frame();
    for (int i = 0; i < 512; i++)
        new_pt_user[i] = ((u64)(i * 0x1000)) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;

    u64 new_pt_user_phys = (u64)phys_of(new_pt_user);
    new_pd_user[0] = new_pt_user_phys | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;

    u64 new_pd_user_phys = (u64)phys_of(new_pd_user);
    new_pdpt[0] = new_pd_user_phys | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;

    return phys_of(new_pdpt);
}

// Framebuffer mapping now takes the full 64-bit physical address directly -
// this is the fix for the >4GB real-hardware framebuffer address.
void paging_map_framebuffer(u64 phys, unsigned int size) {
    unsigned int pages = (size + 0xFFF) / 0x1000;
    for (unsigned int i = 0; i < pages; i++) {
        paging_map_page((unsigned int)pdpt_phys_g, FB_VIRT_BASE + i * 0x1000, phys + (u64)i * 0x1000);
    }
}

unsigned int paging_get_fb_virt() {
    return FB_VIRT_BASE;
}

// Returns the low 32 bits of the requested kernel PD entry, for debug
// printing with your existing serial_hex(unsigned int) helper.
unsigned int paging_get_pd_entry(int index) {
    return (unsigned int)(pd_kernel[index] & 0xFFFFFFFFULL);
}

// New: expose the main PDPT's physical address, since kernel.c needs this
// now instead of the old page_directory-based pd_phys calculation.
unsigned int paging_get_pdpt_phys() {
    return (unsigned int)pdpt_phys_g;
}
