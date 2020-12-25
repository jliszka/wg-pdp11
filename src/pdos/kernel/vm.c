#include "vm.h"

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


void vm_init() {
    volatile int * kernel_par = (int *)KERNEL_PAR;
    volatile int * kernel_pdr = (int *)KERNEL_PDR;
    volatile int * user_par = (int *)USER_PAR;
    volatile int * user_pdr = (int *)USER_PDR;

    *((volatile unsigned int *)MMR0) &= ~VM_ENABLE;

    for (int i = 0; i < 8; i++) {
        kernel_par[i] = 0;
        kernel_pdr[i] = 0;
        user_par[i] = 0;
        user_pdr[i] = 0;
    }

    kernel_par[0] = vm_page_block_number(0);
    kernel_pdr[0] = PDR_READ_WRITE;

    kernel_par[1] = vm_page_block_number(1);
    kernel_pdr[1] = PDR_READ_WRITE;

    kernel_par[2] = vm_page_block_number(2);
    kernel_pdr[2] = PDR_READ_WRITE;

    kernel_par[6] = vm_page_block_number(6);
    kernel_pdr[6] = PDR_READ_WRITE;

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

    for (int i = 0; i < 8; i++) {
        user_par[i] = 0;
        user_pdr[i] = 0;
    }

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
        user_pdr[i] = 0;
    }
}
