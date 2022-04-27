#ifndef VM_H
#define VM_H

#define KERNEL_MAPPING_PAGE 3u
#define KERNEL_MAPPING_PAGE2 4u

#define VM_RO 077404
#define VM_RW 077406

#define VM_PAGE_SIZE 8192
#define vm_page_base_address(page) ((page) << 13)
#define vm_page_block_number(page) ((page) << 7)
#define vm_block_page_number(block) ((block) >> 7)
#define vm_physical_page_address_hi(page) ((page) >> 3)


typedef struct {
    unsigned int lo;
    unsigned int hi;
} unibus_map_t;

void vm_init();
int vm_allocate_page();
int vm_use_page(int page);
void vm_free_page(int page);
void vm_map_kernel_page(int page, unsigned int physical_block_number, int flags);
unsigned int vm_get_kernel_stack_page();
void vm_unmap_kernel_page(int page);
void vm_user_init(unsigned int code_page, unsigned int stack_page);
void vm_user_unmap();

#endif
