#ifndef PAGING_H
#define PAGING_H

void paging_init();
unsigned int paging_alloc_frame();
unsigned int paging_create_directory();
void paging_switch(unsigned int pdpt_phys);
void paging_map_page(unsigned int pdpt_phys, unsigned int virt, unsigned long long phys);
unsigned int paging_alloc_phys_frame();
void paging_map_framebuffer(unsigned long long phys, unsigned int size);
unsigned int paging_get_fb_virt();
unsigned int paging_get_pd_entry(int index);
unsigned int paging_get_pdpt_phys();

#endif
