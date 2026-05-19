#include "include/exec.h"
#include "include/fs.h"
#include "include/vga.h"
#include "include/process.h"

void exec(const char *name) {
    vga_print("exec called\n");
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

    proc->state = PROCESS_STATE_READY;
    if (current_process)
        current_process->state = PROCESS_STATE_READY;
    foreground_pid = proc->pid;
}
