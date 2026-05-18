#ifndef PAGING_H
#define PAGING_H

void paging_init();
unsigned int paging_alloc_frame();
unsigned int *paging_create_directory();
void paging_switch(unsigned int pd_phys);
extern unsigned int page_directory[1024];
#endif
