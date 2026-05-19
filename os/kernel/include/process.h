#ifndef PROCESS_H
#define PROCESS_H

#include "isr.h"

#define MAX_PROCESSES 16
#define KERNEL_STACK_SIZE 4096

#define PROCESS_STATE_FREE 0
#define PROCESS_STATE_READY 1
#define PROCESS_STATE_RUNNING 2
#define PROCESS_STATE_DEAD 3
#define PROCESS_STATE_BLOCKED 4

#define PROCESS_TYPE_USER 0
#define PROCESS_TYPE_KERNEL 1

#define MAX_FDS 16

typedef struct {
    char *data;
    unsigned int size;
    unsigned int offset;
    int used;
} fd_t;

typedef struct {
    unsigned int pid;
    unsigned int state;
    unsigned int type;
    unsigned int esp;
    struct registers regs;
    unsigned int *page_dir;
    unsigned char kernel_stack[KERNEL_STACK_SIZE];
    unsigned int brk;
    fd_t fds[MAX_FDS];
} process_t;

extern process_t processes[MAX_PROCESSES];
extern process_t *current_process;
extern int foreground_pid;

void process_init();
process_t *process_create(void *elf_data);
void process_exit(process_t *proc);
void schedule(struct registers *r);
process_t *process_create_kernel(void (*func)());
void process_start(unsigned int new_esp, unsigned int pd_phys);

#endif
