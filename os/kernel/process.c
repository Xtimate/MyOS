#include "include/process.h"
#include "include/paging.h"
#include "include/elf.h"
#include "include/vga.h"
#include "include/kstring.h"

process_t processes[MAX_PROCESSES];
process_t *current_process = 0;

void process_init() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].state = PROCESS_STATE_FREE;
        processes[i].pid = 0;
    }
}

process_t *process_create(void *elf_data) {
    process_t *proc = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state == PROCESS_STATE_FREE) {
            proc = &processes[i];
            proc->pid = i + 1;
            break;
        }
    }

    if (!proc) {
        vga_print("\nprocess_create: no free slots\n");
        return 0;
    }

    proc->page_dir = paging_create_directory();

    unsigned int entry = elf_load(elf_data);
    if (entry == 0) {
        vga_print("\nprocess_create: ELF load failed\n");
        proc->state = PROCESS_STATE_FREE;
        return 0;
    }

    unsigned int *stack = (unsigned int *)(proc->kernel_stack + KERNEL_STACK_SIZE);

    *--stack = 0x23;
    *--stack = 0x00300000;
    *--stack = 0x202;
    *--stack = 0x1B;
    *--stack = entry;

    *--stack = 0;
    *--stack = 0;

    *--stack = 0;
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;

    *--stack = 0x23;
    *--stack = 0x23;
    *--stack = 0x23;
    *--stack = 0x23;

    proc->esp = (unsigned int)stack;
    proc->state = PROCESS_STATE_READY;

    return proc;
}

void process_exit(process_t *proc) {
    proc->state = PROCESS_STATE_DEAD;
}
