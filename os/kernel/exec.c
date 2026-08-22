#include "include/exec.h"
#include "include/fs.h"
#include "include/kstring.h"
#include "include/vga.h"
#include "include/process.h"
#include "include/input.h"

void exec(const char *name, int argc, char **argv, const char *output_file, const char *input_file) {
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

    if (output_file) {
        fs_create(output_file);
        kstrcpy(proc->fds[1].name, output_file);
        proc->fds[1].mode = 1;
        proc->fds[1].used = 1;
    }

    if (input_file) {
        fs_file_t inf = fs_open(input_file);
        if (inf.data) {
            proc->fds[0].data = inf.data;
            proc->fds[0].size = inf.size;
            proc->fds[0].offset = 0;
            proc->fds[0].mode = 0;
            proc->fds[0].used = 1;
        }
    }

    proc->state = PROCESS_STATE_READY;
    if (current_process)
        current_process->state = PROCESS_STATE_READY;
    foreground_pid = proc->pid;
    input_init();
}
