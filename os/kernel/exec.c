#include "include/exec.h"
#include "include/fs.h"
#include "include/vga.h"
#include "include/process.h"
#include "include/input.h"

void exec(const char *name, int argc, char **argv) {
    vga_print("exec called\n");
    fs_file_t f = fs_open(name);
    if (f.data == 0) {
        vga_print("exec: file not found\n");
        return;
    }

    process_t *proc = process_create(f.data, argc, argv);
    if (proc == 0) {
        vga_print("exec: failed to create process\n");
        return;
    }

    proc->state = PROCESS_STATE_READY;
    if (current_process)
        current_process->state = PROCESS_STATE_READY;
    foreground_pid = proc->pid;
    input_init();
}
