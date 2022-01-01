#include "kmalloc.h"
#include "vm.h"

// 8k bytes per memory page
// divide into 16x 512 byte blocks

#define MEM_BLOCKS 16
#define BYTES_PER_BLOCK 512
#define MEM_BASE vm_page_base_address(2)

static char free_list[MEM_BLOCKS];

void kmalloc_init() {
    for (int i = 0; i < MEM_BLOCKS; i++) {
        free_list[i] = 0;
    }
}

unsigned char * kmalloc() {
    for (int i = 0; i < MEM_BLOCKS; i++) {
        if (free_list[i] == 0) {
            free_list[i] = 1; // TODO: PID?
            return (unsigned char *)(MEM_BASE + BYTES_PER_BLOCK * i);
        }
    }
    return 0;
}

void kfree(unsigned char * mem) {
    if (mem == 0) return;
    int i = (((unsigned int)mem) - MEM_BASE) / BYTES_PER_BLOCK;
    free_list[i] = 0;
}
      
