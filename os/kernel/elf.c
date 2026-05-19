#include "include/elf.h"
#include "include/paging.h"
#include "include/vga.h"
#include "include/kstring.h"

unsigned int elf_load(void *elf_data, unsigned int pd_phys) {
    elf32_header_t *hdr = (elf32_header_t *)elf_data;

    if (hdr->e_magic != ELF_MAGIC) {
        vga_print("ELF: bad magic\n");
        return 0;
    }

    elf32_phdr_t *phdrs = (elf32_phdr_t *)((unsigned char *)elf_data + hdr->e_phoff);

    for (int i = 0; i < hdr->e_phnum; i++) {
        elf32_phdr_t *ph = &phdrs[i];

        if (ph->p_type != PT_LOAD) continue;

        unsigned int virt = ph->p_vaddr & ~0xFFF;
        unsigned int virt_end = (ph->p_vaddr + ph->p_memsz + 0xFFF) & ~0xFFF;

        for (unsigned int v = virt; v < virt_end; v += 4096) {
            unsigned int phys = paging_alloc_phys_frame();
            paging_map_page(pd_phys, v, phys);
        }

        unsigned int old_cr3;
        __asm__ volatile ("mov %%cr3, %0" : "=r"(old_cr3));
        paging_switch(pd_phys);

        unsigned char *src = (unsigned char *)elf_data + ph->p_offset;
        unsigned char *dst = (unsigned char *)ph->p_vaddr;

        kmemcpy(dst, src, ph->p_filesz);

        if (ph->p_memsz > ph->p_filesz)
            kmemset(dst + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
        paging_switch(old_cr3);
    }

    return hdr->e_entry;
}
