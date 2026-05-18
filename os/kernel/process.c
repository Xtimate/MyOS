#include "include/process.h"
#include "include/paging.h"
#include "include/elf.h"
#include "include/vga.h"
#include "include/kstring.h"

#define USER_STACK_BASE 0x00300000
#define USER_STACK_SIZE 0x001000

static unsigned int next_user_stack = USER_STACK_BASE;

static unsigned int alloc_user_stack() {
    unsigned int stack = next_user_stack + USER_STACK_SIZE;
    next_user_stack += USER_STACK_SIZE;
    return stack;
}

process_t processes[MAX_PROCESSES];
process_t *current_process = 0;

void process_init() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].state = PROCESS_STATE_FREE;
        processes[i].pid = 0;
        processes[i].type = PROCESS_TYPE_USER;
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

    proc->type = PROCESS_TYPE_USER;
    proc->page_dir = paging_create_directory();

    unsigned int entry = elf_load(elf_data);
    if (entry == 0) {
        vga_print("\nprocess_create: ELF load failed\n");
        proc->state = PROCESS_STATE_FREE;
        return 0;
    }
    unsigned int user_stack = alloc_user_stack();
    unsigned int *stack = (unsigned int *)(proc->kernel_stack + KERNEL_STACK_SIZE);

    *--stack = 0x23;
    *--stack = user_stack;
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

    extern void process_start_asm();
    *--stack = (unsigned int)process_start_asm;

    proc->esp = (unsigned int)stack;
    proc->state = PROCESS_STATE_READY;

    vga_print("proc kernel stack top: ");
    vga_print_hex((unsigned int)(proc->kernel_stack + KERNEL_STACK_SIZE));
    vga_print("\n");
    return proc;
}

process_t *process_create_kernel(void (*func)()) {
    process_t *proc = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state == PROCESS_STATE_FREE) {
            proc = &processes[i];
            proc->pid = i + 1;
            break;
        }
        }

    if (!proc) {
        vga_print("\nprocess_create_kernel: no free slots\n");
        return 0;
    }

    proc->type = PROCESS_TYPE_KERNEL;
    proc->page_dir = (unsigned int *)(((unsigned int)page_directory) - 0xC0000000);

    unsigned int *stack = (unsigned int *)(proc->kernel_stack + KERNEL_STACK_SIZE);
    *--stack = (unsigned int)func;

    proc->esp = (unsigned int)stack;
    proc->state = PROCESS_STATE_READY;

    return proc;
}

void process_exit(process_t *proc) {
    proc->state = PROCESS_STATE_FREE;
    proc->esp = 0;
    proc->pid = 0;
}
