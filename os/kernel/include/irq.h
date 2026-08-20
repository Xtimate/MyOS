#ifndef IRQ_H
#define IRQ_H

#include "isr.h"

void irq_install();
void irq_install_handler(int irq, void (*handler)(struct registers *r));
void irq_uninstall_handler(int irq);
void irq_clear_mask(unsigned char irq);
#endif
