#include "include/process.h"
#include "include/paging.h"
#include "include/vga.h"
#include "include/gdt.h"

extern void switch_context(unsigned int *old_esp, unsigned int new_esp, unsigned int pd_phys);

void schedule(struct registers *r) {
    if (current_process == 0) {
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (processes[i].state == PROCESS_STATE_READY) {
                current_process = &processes[i];
                current_process->state = PROCESS_STATE_RUNNING;
                break;
            }
        }
        if (current_process == 0) return;
        paging_switch((unsigned int)current_process->page_dir);
        return;
    }

    process_t *next = 0;
    int current_pid = current_process->pid;
    int n = MAX_PROCESSES;

    for (int i = 1; i <= n; i++) {
        int idx = (current_pid - 1 + i) % n;
        if (processes[idx].state == PROCESS_STATE_READY ||
            processes[idx].state == PROCESS_STATE_RUNNING) {
                next = &processes[idx];
                break;
            }
    }

    if (next == 0 || next == current_process) return;

    current_process->regs = *r;
    if (current_process->state == PROCESS_STATE_RUNNING)
        current_process->state = PROCESS_STATE_READY;

    *r = next->regs;

    next->state = PROCESS_STATE_RUNNING;
    current_process = next;

    if (next->type == PROCESS_TYPE_USER) {
        tss_set_kernel_stack((unsigned int)(next->kernel_stack + KERNEL_STACK_SIZE));
    }
    paging_switch((unsigned int)next->page_dir);
}
