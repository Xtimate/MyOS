#ifndef SYSCALL_H
#define SYSCALL_H

#include "isr.h"

#define SYSCALL_PRINT 0
#define SYSCALL_EXIT 1
#define SYSCALL_READ 2
#define SYSCALL_OPEN 3
#define SYSCALL_CLOSE 4
#define SYSCALL_WRITE 5
#define SYSCALL_BRK  6
#define SYSCALL_SLEEP 7

void syscall_handler(struct registers *r);
void syscall_install();

#endif
