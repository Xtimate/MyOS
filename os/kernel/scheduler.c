#include "include/process.h"
#include "include/paging.h"
#include "include/vga.h"

extern void switch_context(unsigned int *old_esp, unsigned int new_esp, unsigned int pd_phys);

void schedule(struct registers *r) {
    if (current_process == 0) return;

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

    process_t *prev = current_process;
    if (prev->state == PROCESS_STATE_RUNNING)
        prev->state = PROCESS_STATE_READY;

    next->state = PROCESS_STATE_RUNNING;
    current_process = next;

    switch_context(&prev->esp, next->esp, (unsigned int)next->page_dir);
}
