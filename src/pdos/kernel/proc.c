#include "proc.h"
#include "stdlib.h"
#include "stdio.h"
#include "load.h"
#include "kmalloc.h"
#include "vm.h"
#include "fs_defs.h"
#include "errno.h"
#include "io.h"

#define MAX_FDS 8
#define MAX_PROCS 16
#define ARGV_BUFSIZE 64

extern int userexec(unsigned int sp);

typedef struct {
    unsigned char code_page;
    unsigned char stack_page;
    unsigned char kernel_stack_page;    
    unsigned int sp;
    fd_t fd_table[MAX_FDS];
} pcb_t;

static pcb_t ptable[MAX_PROCS];
static int cur_pid = -1;

fd_t * proc_fd_alloc(int * fd) {
    if (cur_pid < 0) {
        return 0;
    }

    fd_t * fd_table = ptable[cur_pid].fd_table;

    // Find a free file descriptor entry
    int i;
    for (i = 0; i < MAX_FDS; i++) {
        if (fd_table[i].mode == 0) {
            break;
        }
    }
    if (i == MAX_FDS) {
        // Max fds open
        return 0;
    }

    *fd = i;

    return &fd_table[i];
}

fd_t * proc_fd(int fd) {
    if (cur_pid < 0) {
        return 0;
    }
    return &ptable[cur_pid].fd_table[fd];
}

int proc_create() {
    int i;
    for (i = 0; i < MAX_PROCS; i++) {
        if (ptable[i].code_page == 0) {
            break;
        }
    }
    if (i == MAX_PROCS) {
        return ERR_OUT_OF_PROCS;
    }

    pcb_t * pt = &ptable[i];

    pt->code_page = vm_allocate_page();
    pt->stack_page = vm_allocate_page();
    pt->sp = 0;
    for (int j = 0; j < MAX_FDS; j++) {
        pt->fd_table[j].mode = 0;
    }

    cur_pid = i;

    io_fopen("/dev/tty", 'r');
    io_fopen("/dev/tty", 'w');    
    
    return i;
}

int proc_dup(int pid) {
    int i;
    for (i = 0; i < MAX_PROCS; i++) {
        if (ptable[i].code_page == 0) {
            break;
        }
    }
    if (i == MAX_PROCS) {
        return ERR_OUT_OF_PROCS;
    }

    pcb_t * pt = &ptable[i];
    pcb_t * ppt = &ptable[pid];

    pt->code_page = ppt->code_page;
    pt->stack_page = vm_allocate_page();
    pt->sp = ppt->sp;

    // Copy stack from sp to the end of the page
    unsigned int src_base_address = vm_page_base_address(KERNEL_MAPPING_PAGE);
    vm_map_kernel_page(KERNEL_MAPPING_PAGE, vm_page_block_number(ppt->stack_page));
    unsigned int dst_base_address = vm_page_base_address(KERNEL_MAPPING_PAGE2);
    vm_map_kernel_page(KERNEL_MAPPING_PAGE2, vm_page_block_number(pt->stack_page));

    unsigned int vm_base = vm_page_base_address(7);
    bcopy(
          (unsigned char *)(pt->sp - vm_base + dst_base_address),
          (unsigned char *)(ppt->sp - vm_base + src_base_address),
          VM_PAGE_SIZE - (ppt->sp - vm_base));

    vm_unmap_kernel_page(KERNEL_MAPPING_PAGE);
    vm_unmap_kernel_page(KERNEL_MAPPING_PAGE2);
    
    // Copy file descriptors and associated buffers
    for (int j = 0; j < MAX_FDS; j++) {
        fd_t * fdt = &(pt->fd_table[j]);
        fd_t * pfdt = &(ppt->fd_table[j]);
        if (pfdt->mode) {
            bcopy((unsigned char *)fdt, (unsigned char *)pfdt, sizeof(fd_t));
            if (fdt->buffer) {
                fdt->buffer = kmalloc();
                bcopy(fdt->buffer, pfdt->buffer, BYTES_PER_SECTOR);
            }
        }
    }
}

void proc_free(int pid) {
    vm_free_page(ptable[pid].code_page);
    vm_free_page(ptable[pid].stack_page);
    vm_user_unmap();
}

int proc_exec(int argc, char *argv[]) {
    pcb_t * pt = &ptable[cur_pid];

    int fd = io_fopen(argv[0], 'r');
    if (fd == ERR_FILE_NOT_FOUND) {
        char buf[64];
        strncpy(buf, "/bin/", 6);
        strncpy(buf+5, argv[0], 64-5-1);
        fd = io_fopen(buf, 'r');
    }
    if (fd < 0) {
        println("Command not found");
        return fd;
    }

    int ret = load_file(fd, pt->code_page);

    io_fclose(fd);

    if (ret != 0) return ret;
    
    unsigned int * start_address = (unsigned int *)vm_page_base_address(1);

    // Set up user page tables
    vm_user_init(vm_page_block_number(pt->code_page), vm_page_block_number(pt->stack_page));
    vm_map_kernel_page(KERNEL_MAPPING_PAGE, vm_page_block_number(pt->stack_page));

    // Set up user stack
    unsigned int stack = -ARGV_BUFSIZE - 2 * sizeof(int);
    unsigned int * p = (unsigned int *)(stack + vm_page_base_address(KERNEL_MAPPING_PAGE + 1));
    *p++ = argc;
    *p++ = -ARGV_BUFSIZE;

    // Copy argv to the stack page in user space
    char ** user_argv = (char **)p;
    char * dst = (char *)(user_argv + argc);
    for (int i = 0; i < argc; i++) {
        user_argv[i] = (char *)((dst - (char *)user_argv) - ARGV_BUFSIZE);
        dst = strncpy(dst, argv[i], ARGV_BUFSIZE);
        dst++;
    }

    vm_unmap_kernel_page(KERNEL_MAPPING_PAGE);

    // Call user program main.
    return userexec(stack);
}
