#include "include/process.h"
#include "include/paging.h"
#include "include/elf.h"
#include "include/vga.h"
#include "include/kstring.h"

#define MAX_ARGS 16
#define USER_STACK_SIZE 0x001000
#define USER_STACK_VIRT 0x00300000

process_t processes[MAX_PROCESSES];
process_t *current_process = 0;
int foreground_pid = -1;

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
process_t *process_create(void *elf_data, int argc, char **argv) {
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
    proc->just_started = 1;
    proc->type = PROCESS_TYPE_USER;
    vga_print("alloc dir\n");
    proc->page_dir = (unsigned int *)((unsigned int)page_directory - 0xC0000000);
    vga_print("dir done\n");
    unsigned int pd_phys = (unsigned int)proc->page_dir;
    unsigned int i;
    unsigned int brk = 0;
    vga_print("elf load\n");
    unsigned int entry = elf_load(elf_data, pd_phys, &brk);
    vga_print("elf done\n");
    if (entry == 0) {
        vga_print("process_create: ELF load failed\n");
        proc->state = PROCESS_STATE_FREE;
        return 0;
    }
    proc->brk = brk;
    for (int i = 0; i < MAX_FDS; i++) {
        proc->fds[i].used = 0;
        proc->fds[i].data = 0;
        proc->fds[i].size = 0;
        proc->fds[i].offset = 0;
    }
    unsigned int user_stack = alloc_user_stack(pd_phys);
    unsigned int old_cr3;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(old_cr3));
    paging_switch(pd_phys);
    unsigned int sp = user_stack;
    char *argv_ptrs[MAX_ARGS];
    for (int i = argc - 1; i >= 0; i--) {
        unsigned int len = kstrlen(argv[i]) + 1;
        sp -= len;
        kmemcpy((void *)sp, argv[i], len);
        argv_ptrs[i] = (char *)sp;
    }
    sp &= ~3;
    sp -= 4;
    *(unsigned int *)sp = 0;
    for (int i = argc - 1; i >= 0; i--) {
        sp -= 4;
        *(unsigned int *)sp = (unsigned int)argv_ptrs[i];
    }
    unsigned int argv_ptr = sp;
    sp -= 4;
    *(unsigned int *)sp = argv_ptr;
    sp -= 4;
    *(unsigned int *)sp = (unsigned int)argc;
    sp -= 4;
    *(unsigned int *)sp = 0;
    paging_switch(old_cr3);
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
    proc->regs.useresp = sp;
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
    proc->page_dir = (unsigned int *)paging_get_pdpt_phys();
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
