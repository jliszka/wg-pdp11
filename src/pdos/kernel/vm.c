#include "vm.h"
#include "bitvec.h"

#define MMR0 0177572
#define MMR3 0172516

#define KERNEL_PAR 0172340  // base kernel page address register
#define KERNEL_PDR 0172300  // base kernel page data register
#define USER_PAR 0177640    // base user page address register
#define USER_PDR 0177600    // base user page data register

#define PDR_NON_RESIDENT 0
#define PDR_READ_ONLY 077404
#define PDR_READ_WRITE 077406

#define VM_ENABLE 1
#define ADDRESSING_22_BIT 020

// 22 bits of address space (4MB)
// 13 bits per 8kb page
// 9 bits of pages => 512 8kb pages
// 64 bytes to store a 512-bit bit vector
#define PAGE_BV_MAX 64
static unsigned char free_pages[PAGE_BV_MAX];

int vm_allocate_page() {
    return bitvec_allocate(free_pages, PAGE_BV_MAX);
}

void vm_free_page(int page) {
    bitvec_free(page, free_pages);
}

void vm_init() {

    // Clear the free memory page bit vector.
    for (int i = 0; i < PAGE_BV_MAX; i++) {
        free_pages[i] = 0;
    }
    // Reserve the first 8 pages and the last page for the kernel.
    free_pages[0] = 0xff;
    free_pages[PAGE_BV_MAX-1] = 0x80;

    volatile int * kernel_par = (int *)KERNEL_PAR;
    volatile int * kernel_pdr = (int *)KERNEL_PDR;

    *((volatile unsigned int *)MMR0) &= ~VM_ENABLE;

    // Code
    kernel_par[0] = vm_page_block_number(0);
    kernel_pdr[0] = PDR_READ_ONLY;

    // Code
    kernel_par[1] = vm_page_block_number(1);
    kernel_pdr[1] = PDR_READ_ONLY;

    // Heap
    kernel_par[2] = vm_page_block_number(2);
    kernel_pdr[2] = PDR_READ_WRITE;

    // 3: reserved for mapping between user processes
    kernel_par[3] = 0;
    kernel_pdr[3] = PDR_NON_RESIDENT;

    // 4: reserved for mapping between user processes
    kernel_par[4] = 0;
    kernel_pdr[4] = PDR_NON_RESIDENT;

    // 5: reserved for stack
    kernel_par[5] = 0;
    kernel_pdr[5] = PDR_NON_RESIDENT;

    // Stack
    kernel_par[6] = vm_page_block_number(6);
    kernel_pdr[6] = PDR_READ_WRITE;

    // Unibus
    kernel_par[7] = vm_page_block_number(0777);
    kernel_pdr[7] = PDR_READ_WRITE;

    *((volatile unsigned int *)MMR3) |= ADDRESSING_22_BIT;
    *((volatile unsigned int *)MMR0) |= VM_ENABLE;
}

void vm_map_kernel_page(int page, unsigned int physical_block_number) {
    volatile int * kernel_par = (int *)KERNEL_PAR;
    volatile int * kernel_pdr = (int *)KERNEL_PDR;    
    kernel_par[page] = physical_block_number;
    kernel_pdr[page] = PDR_READ_WRITE;
}

void vm_unmap_kernel_page(int page) {
    volatile int * kernel_par = (int *)KERNEL_PAR;
    volatile int * kernel_pdr = (int *)KERNEL_PDR;    
    kernel_par[page] = 0;
    kernel_pdr[page] = 0;
}

void vm_user_init(unsigned int code_block_number, unsigned int stack_block_number) {
    volatile int * user_par = (int *)USER_PAR;
    volatile int * user_pdr = (int *)USER_PDR;

    vm_user_unmap();

    user_par[1] = code_block_number;
    user_pdr[1] = PDR_READ_WRITE;

    user_par[7] = stack_block_number;
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
