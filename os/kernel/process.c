#include "include/process.h"
#include "include/paging.h"
#include "include/elf.h"
#include "include/vga.h"
#include "include/kstring.h"

process_t processes[MAX_PROCESSES];
process_t *current_process = 0;
int foreground_pid = -1;

#define USER_STACK_SIZE 0x001000
#define USER_STACK_VIRT 0x00300000

static unsigned int alloc_user_stack(unsigned int pd_phys) {
    unsigned int phys = paging_alloc_phys_frame();
    paging_map_page(pd_phys, USER_STACK_VIRT, phys);
    return USER_STACK_VIRT + USER_STACK_SIZE;
}

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
        vga_print("process_create: no free slots\n");
        return 0;
    }

    proc->type = PROCESS_TYPE_USER;
    proc->page_dir = paging_create_directory();
    unsigned int pd_phys = (unsigned int)proc->page_dir;

    unsigned int entry = elf_load(elf_data, pd_phys);

    if (entry == 0) {
        vga_print("process_create: ELF load failed\n");
        proc->state = PROCESS_STATE_FREE;
        return 0;
    }

    unsigned int user_stack = alloc_user_stack(pd_phys);

    proc->regs.gs = 0x23;
    proc->regs.fs = 0x23;
    proc->regs.es = 0x23;
    proc->regs.ds = 0x23;
    proc->regs.edi = 0;
    proc->regs.esi = 0;
    proc->regs.ebp = 0;
    proc->regs.esp = 0;
    proc->regs.ebx = 0;
    proc->regs.edx = 0;
    proc->regs.ecx = 0;
    proc->regs.eax = 0;
    proc->regs.int_no = 0;
    proc->regs.err_code = 0;
    proc->regs.eip = entry;
    proc->regs.cs = 0x1B;
    proc->regs.eflags = 0x202;
    proc->regs.useresp = user_stack;
    proc->regs.ss = 0x23;

    proc->esp = 0;
    proc->state = PROCESS_STATE_READY;

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
        vga_print("process_create_kernel: no free slots\n");
        return 0;
    }

    proc->type = PROCESS_TYPE_KERNEL;
    proc->page_dir = (unsigned int *)((unsigned int)page_directory - 0xC0000000);

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
