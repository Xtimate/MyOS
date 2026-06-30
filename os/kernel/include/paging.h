#ifndef PAGING_H
#define PAGING_H

void paging_init();
unsigned int paging_alloc_frame();
unsigned int *paging_create_directory();
void paging_switch(unsigned int pd_phys);
void paging_map_page(unsigned int pd_phys, unsigned int virt, unsigned int phys);
unsigned int paging_alloc_phys_frame();
void paging_map_framebuffer(unsigned int phys, unsigned int size);
unsigned int paging_get_fb_virt();
unsigned int paging_get_pd_entry(int index);
extern unsigned int page_directory[1024];
#endif
