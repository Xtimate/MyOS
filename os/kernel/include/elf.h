#ifndef ELF_H
#define ELF_H

#define ELF_MAGIC 0x464C457F
#define PT_LOAD 1

typedef struct {
    unsigned int   e_magic;
    unsigned char  e_class;
    unsigned char  e_data;
    unsigned char  e_version;
    unsigned char  e_osabi;
    unsigned char  e_abiversion;
    unsigned char  e_pad[7];
    unsigned short e_type;
    unsigned short e_machine;
    unsigned int   e_version2;
    unsigned int   e_entry;
    unsigned int   e_phoff;
    unsigned int   e_shoff;
    unsigned int   e_flags;
    unsigned short e_ehsize;
    unsigned short e_phentsize;
    unsigned short e_phnum;
    unsigned short e_shentsize;
    unsigned short e_shnum;
    unsigned short e_shstrndx;
} __attribute__((packed)) elf32_header_t;

typedef struct {
    unsigned int p_type;
    unsigned int p_offset;
    unsigned int p_vaddr;
    unsigned int p_paddr;
    unsigned int p_filesz;
    unsigned int p_memsz;
    unsigned int p_flags;
    unsigned int p_align;
} __attribute__((packed)) elf32_phdr_t;

unsigned int elf_load(void *elf_data);

#endif
