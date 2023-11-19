#include "proc.h"
#include "stdlib.h"
#include "stdio.h"
#include "load.h"
#include "kmalloc.h"
#include "vm.h"
#include "fs_defs.h"
#include "fs.h"
#include "errno.h"
#include "io.h"

#define ARGV_BUFSIZE 64
#define MAX_PROCS 16
#define MAX_FDS 16

#define SIG_KILL 9

extern int userexec(unsigned int sp);
extern int kret(unsigned int ksp, unsigned int kernel_stack_page, unsigned int * kspp);
extern unsigned int pwd;

static pcb_t ptable[MAX_PROCS];
static fd_t fd_table[MAX_FDS];
static int cur_pid = -1;

void proc_init() {
    for (int i = 0; i < MAX_FDS; i++) {
        fd_table[i].refcount = 0;
    }
    for (int i = 0; i < MAX_PROCS; i++) {
        ptable[i].state = 0;
    }
}

int proc_fd_alloc(fd_t ** fdt_out) {
    if (cur_pid < 0) {
        return -1;
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

void proc_fd_free(int fd, int pid) {
    if (pid == -1) {
        pid = cur_pid;
    }
    fd_t * fdt = ptable[pid].fds[fd];
    if (fdt == 0) {
        // Already freed? Maybe error here.
        return;
    }
    fdt->refcount--;
    if (fdt->refcount == 0) {
        if (fdt->pipe_fdt != 0) {
            fd_t * pfdt = fdt->pipe_fdt;
            if (pfdt->refcount == 0) {
                fdt->mode = 0;
                pfdt->mode = 0;
                kfree(fdt->buffer);
                fdt->buffer = 0;
                pfdt->buffer = 0;
                fdt->pipe_fdt = 0;
                pfdt->pipe_fdt = 0;
            }
        } else {
            fdt->mode = 0;
            kfree(fdt->buffer);
            fdt->buffer = 0;
            fdt->pipe_fdt = 0;
        }
    }
    ptable[pid].fds[fd] = 0;
}

void proc_fd_assign(fd_t * fdt, int fd) {
    ptable[cur_pid].fds[fd] = fdt;
    fdt->refcount++;
}

void _proc_free_fds(pcb_t * pt, int pid) {
    for (int i = 0; i < MAX_PROC_FDS; i++) {
        if (pt->fds[i] != 0) {
            proc_fd_free(i, pid);
        }
    }
}

fd_t * proc_fd(int fd) {
    if (cur_pid < 0) {
        return 0;
    }
    return ptable[cur_pid].fds[fd];
}

int proc_cwd() {
    if (cur_pid < 0) {
        return pwd;
    }
    return ptable[cur_pid].cwd;
}

int proc_create() {
    int i;
    for (i = 0; i < MAX_PROCS; i++) {
        if (ptable[i].state == 0) {
            break;
        }
    }
    if (i == MAX_PROCS) {
        return ERR_OUT_OF_PROCS;
    }

    pcb_t * pt = &ptable[i];

    pt->code_page = 0;
    pt->stack_page = vm_allocate_page();
    pt->kernel_stack_page = vm_use_page(vm_get_kernel_stack_page());
    pt->ksp = 0;
    pt->exit_code = 0;
    pt->signal = 0;
    pt->flags = PROC_FLAG_HAS_TTY_IN | PROC_FLAG_HAS_TTY_OUT;
    pt->state = PROC_STATE_RUNNABLE;
    pt->ppid = -1;
    pt->cwd = ROOT_DIR_INODE;

    cur_pid = i;

    // Initialize stdin and stdout
    io_open("/dev/tty", 'r');
    io_open("/dev/tty", 'w');

    return i;
}

int _proc_load(char * path, int code_page) {
    int fd = io_open(path, 'r');
    if (fd == ERR_FILE_NOT_FOUND && path[0] != '/') {
        // TODO: resolve path in the shell
        char buf[64];
        strncpy(buf, "/bin/", 6);
        strncpy(buf+5, path, 64-5-1);
        fd = io_open(buf, 'r');
    }
    if (fd < 0) {
        println("Command not found");
        return fd;
    }

    int ret = load_file(fd, code_page);

    io_close(fd);

    return ret;
}

unsigned int _proc_init_stack(int argc, char * argv[], int stack_page) {
    vm_map_kernel_page(KERNEL_MAPPING_PAGE, stack_page, VM_RW);

    // Set up user stack
    unsigned int stack = -ARGV_BUFSIZE - 2 * sizeof(int);
    unsigned int base_address = vm_page_base_address(KERNEL_MAPPING_PAGE + 1);
    unsigned int * p = (unsigned int *)(stack + base_address);
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

    return stack;
}

int _proc_cleanup(int pid, int exit_code) {
    pcb_t * pt = &ptable[pid];

    vm_free_page(pt->code_page);
    vm_free_page(pt->stack_page);
    vm_free_page(pt->kernel_stack_page);
    pt->code_page = 0;
    pt->stack_page = 0;
    pt->kernel_stack_page = 0;
    pt->ksp = 0;
    pt->cwd = 0;
    pt->exit_code = 0;

    _proc_free_fds(pt, pid);

    if (pt->ppid == -1) {
        // Program has exited and has no parent.
        pt->state = 0;
    } else {
        pt->state = PROC_STATE_EXITED;
        pt->exit_code = exit_code;
        // If the parent was waiting for us to exit,
        // make the parent runnable.
        pcb_t * ppt = &ptable[pt->ppid];
        if (ppt->state == PROC_STATE_WAIT) {
            ppt->state = PROC_STATE_RUNNABLE;
        }
    }
}

int proc_exec(int argc, char *argv[]) {
    pcb_t * pt = &ptable[cur_pid];

    if (pt->code_page != 0) {
        vm_free_page(pt->code_page);
    }
    pt->code_page = vm_allocate_page();

    // Load program into memory
    int ret = _proc_load(argv[0], pt->code_page);
    if (ret != 0) return ret;

    // Set up user page tables
    vm_user_init(pt->code_page, pt->stack_page);

    // Set up the stack
    int stack = _proc_init_stack(argc, argv, pt->stack_page);

    // Call user program main.
    int exit_code = userexec(stack);

    // Flush stdout
    io_fsync(1);

    // Need to reload pt because we might have switched contexts
    _proc_cleanup(cur_pid, exit_code);

    return proc_switch();
}


int proc_dup(unsigned int sp, unsigned int ksp) {
    int child_pid;
    for (child_pid = 0; child_pid < MAX_PROCS; child_pid++) {
        if (ptable[child_pid].state == 0) {
            break;
        }
    }
    if (child_pid == MAX_PROCS) {
        return ERR_OUT_OF_PROCS;
    }

    pcb_t * pt = &ptable[child_pid];
    pcb_t * ppt = &ptable[cur_pid];

    pt->code_page = vm_use_page(ppt->code_page);
    pt->stack_page = vm_allocate_page();
    pt->kernel_stack_page = vm_allocate_page();
    pt->ksp = ksp;
    pt->exit_code = 0;
    pt->signal = 0;
    pt->flags = ppt->flags;
    pt->state = PROC_STATE_RUNNABLE;
    pt->ppid = cur_pid;
    pt->cwd = ppt->cwd;

    // Copy file descriptors
    for (int i = 0; i < MAX_PROC_FDS; i++) {
        if (ppt->fds[i] != 0) {
            ppt->fds[i]->refcount++;
            pt->fds[i] = ppt->fds[i];
        }
    }

    // Copy user stack from sp to the end of the page
    unsigned int src_base_address = vm_page_base_address(KERNEL_MAPPING_PAGE);
    vm_map_kernel_page(KERNEL_MAPPING_PAGE, ppt->stack_page, VM_RO);
    unsigned int dst_base_address = vm_page_base_address(KERNEL_MAPPING_PAGE2);
    vm_map_kernel_page(KERNEL_MAPPING_PAGE2, pt->stack_page, VM_RW);

    unsigned int vm_base = vm_page_base_address(7);
    bcopy(
          (unsigned char *)(sp - vm_base + dst_base_address),
          (unsigned char *)(sp - vm_base + src_base_address),
          VM_PAGE_SIZE - (sp - vm_base));

    vm_unmap_kernel_page(KERNEL_MAPPING_PAGE);
    vm_unmap_kernel_page(KERNEL_MAPPING_PAGE2);

    // Copy kernel stack from ksp to the end of the the page
    dst_base_address = vm_page_base_address(KERNEL_MAPPING_PAGE);
    vm_map_kernel_page(KERNEL_MAPPING_PAGE, pt->kernel_stack_page, VM_RW);

    vm_base = vm_page_base_address(KERNEL_STACK_PAGE);
    bcopy(
          (unsigned char *)(ksp - vm_base + dst_base_address),
          (unsigned char *)ksp,
          VM_PAGE_SIZE - (ksp - vm_base));
    
    vm_unmap_kernel_page(KERNEL_MAPPING_PAGE);

    return child_pid;
}


int proc_get_flag(int flag) {
    pcb_t * pt = &ptable[cur_pid];
    return (pt->flags & flag) != 0 ? 1 : 0;
}

void proc_set_flag(int flag) {
    pcb_t * pt = &ptable[cur_pid];
    pt->flags |= flag;
}

void proc_clear_flag(int flag) {
    pcb_t * pt = &ptable[cur_pid];
    pt->flags &= ~flag;
}

int _proc_select() {
    // Find a new process to run.
    for (int i = 1; i <= MAX_PROCS; i++) {
        int pid = (cur_pid + i) % MAX_PROCS;
        int state = ptable[pid].state;
        if (state == PROC_STATE_RUNNABLE) {
            return pid;
        }
    }

    asm("wait");

    // If none found, see if any of the IO blocked procs are ready to wake up
    for (int i = 1; i <= MAX_PROCS; i++) {
        int pid = (cur_pid + i) % MAX_PROCS;
        if (ptable[pid].state == PROC_STATE_IO_BLOCKED) {
            ptable[pid].state = PROC_STATE_RUNNABLE;
            return pid;
        }
    }

    return MAX_PROCS;
}

int proc_switch() {
    // Handle any signals
    for (int pid = 0; pid < MAX_PROCS; pid++) {
        if (ptable[pid].signal != 0) {
            if (ptable[pid].signal == SIG_KILL) {
                _proc_cleanup(pid, SIG_KILL);
            }
        }
    }

    int new_pid = MAX_PROCS;
    while (new_pid == MAX_PROCS) {
        new_pid = _proc_select();
    }

    if (new_pid == cur_pid) {
        // Don't try to switch from current pid to itself,
        // just return!
        return 0;
    }

    if (new_pid == MAX_PROCS) {
        // No procs to run. This shouldn't happen, but exit
        // to the root shell.
        // TODO: asm("wait");
        cur_pid = -1;
        return -1;
    }

    pcb_t * pt = &ptable[cur_pid];
    pcb_t * new_pt = &ptable[new_pid];
    cur_pid = new_pid;

    vm_user_init(new_pt->code_page, new_pt->stack_page);
    kret(new_pt->ksp, new_pt->kernel_stack_page, &(pt->ksp));

    return 0;
}

int proc_block() {
    ptable[cur_pid].state = PROC_STATE_IO_BLOCKED;
    return proc_switch();
}

int proc_read_block(fd_t * fdt) {
    pcb_t * pt = &ptable[cur_pid];
    pt->state = PROC_STATE_BLOCKED;
    pt->next = fdt->read_wait;
    fdt->read_wait = pt;
    return proc_switch();
}

int proc_write_block(fd_t * fdt) {
    pcb_t * pt = &ptable[cur_pid];
    pt->state = PROC_STATE_BLOCKED;
    pt->next = fdt->write_wait;
    fdt->write_wait = pt;
    return proc_switch();
}

int proc_wait(int pid) {
    volatile pcb_t * pt = &ptable[pid];
    volatile pcb_t * ppt = &ptable[cur_pid];
    if (pt->state == 0) {
        return -1; // TODO: better error code
    }

    while (pt->state != PROC_STATE_EXITED) {
        ppt->state = PROC_STATE_WAIT;
        proc_switch();
    }

    int ret = pt->exit_code;
    pt = &ptable[pid];
    pt->state = 0;
    pt->exit_code = 0;
    pt->ppid = -1;

    return ret;
}

int proc_chdir(char * path) {
    path_info_t path_info;
    int ret = fs_resolve_path(path, &path_info);
    if (ret < 0) {
        return ret;
    }
    if (!fs_is_dir(path_info.inode)) {
        return ERR_NOT_A_DIRECTORY;
    }

    ptable[cur_pid].cwd = path_info.inode;
    return 0;
}

int proc_getcwd(char * path, int len) {
    char * end = fs_build_path(ptable[cur_pid].cwd, path, len);
    *end = 0;
    return (int)(end - path + 1);
}

int proc_kill(int pid, int signal) {
    ptable[pid].signal = signal;
    return 0;
}
