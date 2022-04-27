#include "proc.h"
#include "stdlib.h"
#include "stdio.h"
#include "load.h"
#include "kmalloc.h"
#include "vm.h"
#include "fs_defs.h"
#include "errno.h"
#include "io.h"

#define ARGV_BUFSIZE 64
#define MAX_PROCS 16
#define MAX_FDS 32
#define MAX_PROC_FDS 8

extern int userexec(unsigned int sp);

typedef struct {
    unsigned char code_page;
    unsigned char stack_page;
    unsigned char kernel_stack_page;    
    unsigned int sp;
    fd_t * fds[MAX_PROC_FDS];
} pcb_t;

static pcb_t ptable[MAX_PROCS];
static fd_t fd_table[MAX_FDS];
static int cur_pid = -1;

void proc_init() {
    for (int i = 0; i < MAX_FDS; i++) {
        fd_table[i].refcount = 0;
    }
}

int proc_fd_alloc(fd_t ** fdt_out) {
    if (cur_pid < 0) {
        return 0;
    }

    // Find a free file descriptor table entry
    int i;
    for (i = 0; i < MAX_FDS; i++) {
        if (fd_table[i].refcount == 0) {
            break;
        }
    }
    if (i == MAX_FDS) {
        // Max fds open
        return ERR_OUT_OF_FDS;
    }

    fd_t * fdt = &fd_table[i];
    fdt->refcount = 1;

    pcb_t * pt = &ptable[cur_pid];

    // Find a spot in the process's fd table
    int fd;
    for (fd = 0; fd < MAX_PROC_FDS; fd++) {
        if (pt->fds[fd] == 0) {
            break;
        }
    }
    if (fd == MAX_PROC_FDS) {
        return ERR_OUT_OF_FDS;
    }

    pt->fds[fd] = fdt;
    *fdt_out = fdt;

    return fd;
}

void proc_fd_free(int fd) {
    fd_t * fdt = ptable[cur_pid].fds[fd];
    fdt->refcount--;
    if (fdt->refcount == 0) {
        if (fdt->buffer != 0) {
            kfree(fdt->buffer);
            fdt->buffer = 0;
        }
    }
    ptable[cur_pid].fds[fd] = 0;
}

fd_t * proc_fd(int fd) {
    if (cur_pid < 0) {
        return 0;
    }
    return ptable[cur_pid].fds[fd];
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
    pt->kernel_stack_page = 6;
    pt->sp = 0;
    for (int i = 0; i < MAX_PROC_FDS; i++) {
        pt->fds[i] = 0;
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
    
    // Copy file descriptors
    for (int i = 0; i < MAX_PROC_FDS; i++) {
        if (ppt->fds[i] != 0) {
            ppt->fds[i]->refcount++;
            pt->fds[i] = ppt->fds[i];
        }
    }
}

void proc_free(int pid) {
    pcb_t * pt = &ptable[pid];
    vm_free_page(pt->code_page);
    vm_free_page(pt->stack_page);
    pt->code_page = 0;
    pt->stack_page = 0;
    vm_user_unmap();

    for (int i = 0; i < MAX_PROC_FDS; i++) {
        if (pt->fds[i] != 0) {
            proc_fd_free(i);
        }
    }
}

int proc_exec(int argc, char *argv[]) {
    pcb_t * pt = &ptable[cur_pid];

    int fd = io_fopen(argv[0], 'r');
    if (fd == ERR_FILE_NOT_FOUND && argv[0][0] != '/') {
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
