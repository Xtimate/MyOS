#ifndef SYSCALL_H
#define SYSCALL_H

#include "isr.h"

#define SYSCALL_PRINT 0
#define SYSCALL_EXIT 1

void syscall_handler(struct registers *r);
void syscall_install();

#endif
