#include "include/exec.h"
#include "include/fs.h"
#include "include/elf.h"
#include "include/vga.h"
#include "include/usermode.h"

#define USER_STACK_TOP 0x00300000

void exec(const char *name) {
    fs_file_t f = fs_open(name);
    if (f.data == 0) {
        vga_print("exec: file not found\n");
        return;
    }

    unsigned int entry = elf_load(f.data);
    if (entry == 0) {
        vga_print("exec: ELF load failed\n");
        return;
    }

    jump_usermode((void (*)())entry, USER_STACK_TOP);
}
