#include "include/elf.h"
#include "include/vga.h"
#include "include/kstring.h"

unsigned int elf_load(void *elf_data) {
    elf32_header_t *hdr = (elf32_header_t *)elf_data;

    if (hdr->e_magic != ELF_MAGIC) {
        vga_print("ELF: bad magic\n");
        return 0;
    }

    elf32_phdr_t *phdrs = (elf32_phdr_t *)((unsigned char *)elf_data + hdr->e_phoff);

    for (int i = 0; i < hdr->e_phnum; i++) {
        elf32_phdr_t *ph = &phdrs[i];

        if (ph->p_type != PT_LOAD) continue;

        unsigned char *src = (unsigned char *)elf_data + ph->p_offset;
        unsigned char *dst = (unsigned char *)ph->p_vaddr;

        kmemcpy(dst, src, ph->p_filesz);

        if (ph->p_memsz > ph->p_filesz)
            kmemset(dst + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
    }

    return hdr->e_entry;
}
