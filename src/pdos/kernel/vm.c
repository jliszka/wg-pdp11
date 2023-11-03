#include "vm.h"
#include "bitvec.h"

#define MMR0 0177572
#define MMR3 0172516

#define KERNEL_PAR 0172340  // base kernel page address register
#define KERNEL_PDR 0172300  // base kernel page data register
#define USER_PAR 0177640    // base user page address register
#define USER_PDR 0177600    // base user page data register
#define UNIBUS_MAP 0170200  // base unibus map register

#define PDR_NON_RESIDENT 0
#define PDR_READ_ONLY 077404
#define PDR_READ_WRITE 077406

#define VM_ENABLE 1
#define ADDRESSING_22_BIT 020
#define UNIBUS_MAP_ENABLE 040

// 22 bits of address space (4MB)
// 13 bits per 8kb page
// 9 bits of pages => 512 8kb pages
// We'll hand out 64 of them with a 1-byte refcount.
#define PAGE_MAX 64
static unsigned char free_pages[PAGE_MAX];

int vm_allocate_page() {
    for (int i = 0; i < PAGE_MAX; i++) {
        if (free_pages[i] == 0) {
            return vm_use_page(i);
        }
    }
    return -1;
}

int vm_use_page(int page) {
    free_pages[page]++;
    return page;
}

void vm_free_page(int page) {
    if (free_pages[page] > 0) {
        free_pages[page]--;
    }
}

void vm_init() {

    // Clear the free memory page map.
    for (int i = 0; i < PAGE_MAX; i++) {
        // Reserve the first 8 pages for the kernel.
        free_pages[i] = i < 8 ? 1 : 0;
    }

    *((volatile unsigned int *)MMR0) &= ~VM_ENABLE;

    // Code
    vm_map_kernel_page(0, 0, PDR_READ_ONLY);
    vm_map_kernel_page(1, 1, PDR_READ_ONLY);
    vm_map_kernel_page(2, 2, PDR_READ_ONLY);

    // Heap
    vm_map_kernel_page(3, 3, PDR_READ_WRITE);

    // reserved for mapping between user processes
    vm_map_kernel_page(4, 0, PDR_NON_RESIDENT);
    vm_map_kernel_page(5, 0, PDR_NON_RESIDENT);

    // Stack
    vm_map_kernel_page(6, 6, PDR_READ_WRITE);

    // Unibus
    vm_map_kernel_page(7, 0777, PDR_READ_WRITE);

    *((volatile unsigned int *)MMR3) |= ADDRESSING_22_BIT;
    *((volatile unsigned int *)MMR3) |= UNIBUS_MAP_ENABLE;
    *((volatile unsigned int *)MMR0) |= VM_ENABLE;
}

unsigned int vm_get_kernel_stack_page() {
    volatile int * kernel_par = (int *)KERNEL_PAR;
    return vm_block_page_number(kernel_par[KERNEL_STACK_PAGE]);
}

void vm_map_kernel_page(int virtual_page, unsigned int physical_page, int flags) {
    volatile int * kernel_par = (int *)KERNEL_PAR;
    volatile int * kernel_pdr = (int *)KERNEL_PDR;    
    kernel_par[virtual_page] = vm_page_block_number(physical_page);
    kernel_pdr[virtual_page] = flags;

    volatile unibus_map_t * unibus_map = (unibus_map_t *)UNIBUS_MAP;
    unibus_map[virtual_page].lo = vm_page_base_address(physical_page);
    unibus_map[virtual_page].hi = vm_physical_page_address_hi(physical_page);
}

void vm_unmap_kernel_page(int page) {
    volatile int * kernel_par = (int *)KERNEL_PAR;
    volatile int * kernel_pdr = (int *)KERNEL_PDR;
    kernel_par[page] = 0;
    kernel_pdr[page] = 0;
}

void vm_user_init(unsigned int code_page, unsigned int stack_page) {
    volatile int * user_par = (int *)USER_PAR;
    volatile int * user_pdr = (int *)USER_PDR;

    vm_user_unmap();

    user_par[1] = vm_page_block_number(code_page);
    user_pdr[1] = PDR_READ_WRITE;

    user_par[7] = vm_page_block_number(stack_page);
    user_pdr[7] = PDR_READ_WRITE;
}

void vm_user_unmap() {
    volatile int * user_par = (int *)USER_PAR;
    volatile int * user_pdr = (int *)USER_PDR;

    for (int i = 0; i < 8; i++) {
        user_par[i] = 0;
        user_pdr[i] = PDR_NON_RESIDENT;
    }
}
