#include "kmalloc.h"
#include "vga.h"

struct block_header {
    unsigned int size;
    unsigned int free;
    struct block_header *next;
};

#define HEADER_SIZE sizeof(struct block_header)

static struct block_header *heap_start = 0;
static unsigned int heap_end = 0;

void kmalloc_init(unsigned int start, unsigned int end) {
    heap_start = (struct block_header *)start;
    heap_start->size = end - start - HEADER_SIZE;
    heap_start->free = 1;
    heap_start->next = 0;
    heap_end = end;
}

void *kmalloc(unsigned int size) {
    // align to 4 bytes
    if (size % 4 != 0) {
        size += 4 - (size % 4);
    }

    struct block_header *cur = heap_start;

    while (cur) {
        if (cur->free && cur->size >= size) {
            // split block if it's big enough
            if (cur->size >= size + HEADER_SIZE + 4) {
                struct block_header *new = (struct block_header *)((unsigned int)cur + HEADER_SIZE + size);
                new->size = cur->size - size - HEADER_SIZE;
                new->free = 1;
                new->next = cur->next;
                cur->next = new;
                cur->size = size;
            }
            cur->free = 0;
            return (void *)((unsigned int)cur + HEADER_SIZE);
        }
        cur = cur->next;
    }

    return 0;  // out of memory
}

void kfree(void *ptr) {
    if (!ptr) return;

    struct block_header *block = (struct block_header *)((unsigned int)ptr - HEADER_SIZE);
    block->free = 1;

    // merge adjacent free blocks
    struct block_header *cur = heap_start;
    while (cur && cur->next) {
        if (cur->free && cur->next->free) {
            cur->size += HEADER_SIZE + cur->next->size;
            cur->next = cur->next->next;
        } else {
            cur = cur->next;
        }
    }
}

void kmalloc_info() {
    vga_print("\nHeap start: ");
    vga_print_hex((unsigned int)heap_start);
    vga_print("\nHeap end:   ");
    vga_print_hex(heap_end);
    vga_print("\n");

    int block_num = 0;
    unsigned int total_used = 0;
    unsigned int total_free = 0;

    struct block_header *cur = heap_start;
    while (cur) {
        vga_print("  [");
        vga_print_num(block_num++);
        vga_print("] addr=");
        vga_print_hex((unsigned int)cur);
        vga_print(" size=");
        vga_print_num(cur->size);
        vga_print(cur->free ? "  FREE\n" : "  USED\n");

        if (cur->free) total_free += cur->size;
        else total_used += cur->size;

        cur = cur->next;
    }

    vga_print("Total used: ");
    vga_print_num(total_used);
    vga_print(" bytes\nTotal free: ");
    vga_print_num(total_free);
    vga_print(" bytes\n");
}

unsigned int kmalloc_used() {
    unsigned int used = 0;
    struct block_header *cur = heap_start;
    while (cur) {
        if (!cur->free) used += cur->size;
        cur = cur->next;
    }
    return used;
}

unsigned int kmalloc_free() {
    unsigned int free = 0;
    struct block_header *cur = heap_start;
    while (cur) {
        if (cur->free) free += cur->size;
        cur = cur->next;
    }
    return free;
}
