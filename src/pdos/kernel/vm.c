#include "vm.h"
#include "bitvec.h"

#define MMR0 0177572
#define MMR3 0172516

#define KERNEL_IPDR 0172300  // base kernel I-space page data register
#define KERNEL_DPDR 0172320  // base kernel D-space page data register
#define KERNEL_IPAR 0172340  // base kernel I-space page address register
#define KERNEL_DPAR 0172360  // base kernel D-space page address register
#define USER_PDR 0177600    // base user page data register
#define USER_PAR 0177640    // base user page address register
#define UNIBUS_MAP 0170200  // base unibus map register

#define PDR_NON_RESIDENT 0
#define PDR_READ_ONLY 077404
#define PDR_READ_WRITE 077406

#define VM_ENABLE 1
#define ADDRESSING_22_BIT 020
#define UNIBUS_MAP_ENABLE 040
#define KERNEL_D_SPACE_ENABLE 04

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

    volatile int * kernel_ipar = (int *)KERNEL_IPAR;
    volatile int * kernel_ipdr = (int *)KERNEL_IPDR;
    volatile int * kernel_dpar = (int *)KERNEL_DPAR;
    volatile int * kernel_dpdr = (int *)KERNEL_DPDR;

    *((volatile unsigned int *)MMR0) &= ~VM_ENABLE;

    // I-SPACE

    // 0: Code
    kernel_ipar[0] = vm_page_block_number(0);
    kernel_ipdr[0] = PDR_READ_ONLY;

    // 1: Code
    kernel_ipar[1] = vm_page_block_number(1);
    kernel_ipdr[1] = PDR_READ_ONLY;

    // 2: Code
    kernel_ipar[2] = vm_page_block_number(2);
    kernel_ipdr[2] = PDR_READ_ONLY;

    // 3-7: unmapped
    kernel_ipar[3] = 0;
    kernel_ipdr[3] = PDR_NON_RESIDENT;

    kernel_ipar[4] = 0;
    kernel_ipdr[4] = PDR_NON_RESIDENT;

    kernel_ipar[5] = 0;
    kernel_ipdr[5] = PDR_NON_RESIDENT;

    kernel_ipar[6] = 0;
    kernel_ipdr[6] = PDR_NON_RESIDENT;

    kernel_ipar[7] = 0;
    kernel_ipdr[7] = PDR_NON_RESIDENT;

    // D-SPACE

    // 0: Data segment
    // This is loaded at page 5 in physical memory space by convention,
    // but the linker is told it's based at page 0.
    //
    // TODO: leave this unmapped so that null pointer references fault?
    kernel_dpar[0] = vm_page_block_number(5);
    kernel_dpdr[0] = PDR_READ_WRITE;

    // 1: Data segment
    kernel_dpar[1] = vm_page_block_number(6);
    kernel_dpdr[1] = PDR_READ_WRITE;

    // 2: Heap
    kernel_dpar[2] = vm_page_block_number(3);
    kernel_dpdr[2] = PDR_READ_WRITE;

    // 3: reserved for future heap
    kernel_dpar[3] = 0;
    kernel_dpdr[3] = PDR_NON_RESIDENT;

    // 4: reserved for mapping between user processes
    kernel_dpar[4] = 0;
    kernel_dpdr[4] = PDR_NON_RESIDENT;

    // 5: reserved for stack
    kernel_dpar[5] = 0;
    kernel_dpdr[5] = PDR_NON_RESIDENT;

    // 6: Stack
    kernel_dpar[6] = vm_page_block_number(4);
    kernel_dpdr[6] = PDR_READ_WRITE;

    // 7: Unibus
    kernel_dpar[7] = vm_page_block_number(0777);
    kernel_dpdr[7] = PDR_READ_WRITE;

    *((volatile unsigned int *)MMR3) |= ADDRESSING_22_BIT;
    *((volatile unsigned int *)MMR3) |= KERNEL_D_SPACE_ENABLE;
    *((volatile unsigned int *)MMR0) |= VM_ENABLE;

    // !!!!!
    // Now it is safe to write to globals!
    // !!!!!

    // Clear the free memory page bit vector.
    for (int i = 0; i < PAGE_BV_MAX; i++) {
        free_pages[i] = 0;
    }
    // Reserve the first 8 pages and the last page for the kernel.
    free_pages[0] = 0xff;
    free_pages[PAGE_BV_MAX-1] = 0x80;
}

unsigned int vm_get_kernel_stack_page() {
    volatile int * kernel_par = (int *)KERNEL_DPAR;
    return vm_block_page_number(kernel_par[KERNEL_STACK_PAGE]);
}

int vm_map_kernel_page(int virtual_page, unsigned int physical_page, int flags) {
    volatile int * kernel_par = (int *)KERNEL_DPAR;
    volatile int * kernel_pdr = (int *)KERNEL_DPDR;
    int old_physical_block_number = kernel_par[virtual_page];

    kernel_par[virtual_page] = vm_page_block_number(physical_page);
    kernel_pdr[virtual_page] = flags;

    volatile unibus_map_t * unibus_map = (unibus_map_t *)UNIBUS_MAP;
    unibus_map[virtual_page].lo = vm_page_base_address(physical_page);
    unibus_map[virtual_page].hi = vm_physical_page_address_hi(physical_page);

    return old_physical_block_number;
}

void vm_unmap_kernel_page(int page) {
    volatile int * kernel_par = (int *)KERNEL_DPAR;
    volatile int * kernel_pdr = (int *)KERNEL_DPDR;
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
