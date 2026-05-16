#ifndef KMALLOC_H
#define KMALLOC_H

void kmalloc_init(unsigned int start, unsigned int end);
void *kmalloc(unsigned int size);
void kfree(void *ptr);
void kmalloc_info();
unsigned int kmalloc_used();
unsigned int kmalloc_free();

#endif
