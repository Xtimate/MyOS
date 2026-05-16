#include "kmalloc.h"

static unsigned int heap_start = 0;
static unsigned int heap_end = 0;
static unsigned int heap_cur = 0;

void kmalloc_init(unsigned int start, unsigned int end) {
    heap_start = start;
    heap_end = end;
    heap_cur = start;
}

void *kmalloc(unsigned int size) {
    if (size % 4 != 0) {
        size += 4 - (size % 4);
    }

    if (heap_cur + size > heap_end) {
        return 0;
    }

    unsigned int addr = heap_cur;
    heap_cur += size;
    return (void *)addr;
}

unsigned int kmalloc_used() {
    return heap_cur - heap_start;
}

unsigned int kmalloc_free() {
    return heap_end - heap_cur;
}
