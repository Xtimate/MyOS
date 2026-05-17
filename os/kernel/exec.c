#include "include/exec.h"
#include "include/fs.h"
#include "include/vga.h"
#include "include/process.h"

extern void process_start(unsigned int new_esp, unsigned int pd_phys);

void exec(const char *name) {
    fs_file_t f = fs_open(name);
    if (f.data == 0) {
        vga_print("exec: file not found\n");
        return;
    }

    process_t *proc = process_create(f.data);
    if (proc == 0) {
        vga_print("exec: failed to create process\n");
        return;
    }

    current_process = proc;
    proc->state = PROCESS_STATE_RUNNING;

    process_start(proc->esp, (unsigned int)proc->page_dir);
}
