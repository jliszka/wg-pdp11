#ifndef VM_H
#define VM_H

#define vm_page_base_address(page) ((page) << 13)
#define vm_page_block_number(page) ((page) << 7)

void vm_init();
void vm_map_kernel_page(int page, unsigned int physical_block_number);
void vm_unmap_kernel_page(int page);
void vm_user_init(unsigned int code_block_number, unsigned int stack_block_number);

#endif
