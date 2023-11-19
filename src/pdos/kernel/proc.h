#ifndef PROC_H
#define PROC_H

#include "io.h"

#define PROC_FLAG_HAS_TTY_IN 1
#define PROC_FLAG_HAS_TTY_OUT 2

#define PROC_STATE_RUNNABLE 1
#define PROC_STATE_WAIT 2
#define PROC_STATE_EXITED 3
#define PROC_STATE_BLOCKED 4
#define PROC_STATE_IO_BLOCKED 5

#define MAX_PROC_FDS 8

typedef struct pcb_t {
    unsigned char code_page;
    unsigned char stack_page;
    unsigned char kernel_stack_page;
    unsigned int ksp;
    int exit_code;
    int state;
    int ppid;
    int cwd;
    int signal;
    int flags;
    struct fd_t * fds[MAX_PROC_FDS];
    struct pcb_t * next;
} pcb_t;

typedef struct fd_t {
    int inode;
    int cur_block;
    int pos;
    int max_pos;
    char mode;
    char refcount;
    struct fd_t * pipe_fdt;
    vfile_t * vfile;
    unsigned char * buffer;
    pcb_t * read_wait;
    pcb_t * write_wait;
} fd_t;

void proc_init();
int proc_fd_alloc(fd_t ** fdt);
void proc_fd_free(int fd, int pid);
void proc_fd_assign(fd_t * fdt, int fd);
fd_t * proc_fd(int fd);
int proc_cwd();
int proc_create();
int proc_dup(unsigned int sp, unsigned int ksp);
void proc_free(int pid);
int proc_exec(int argc, char ** argv);
int proc_switch();
int proc_chdir(char * path);
int proc_getcwd(char * path, int len);
int proc_block();
int proc_read_block(fd_t * fdt);
int proc_write_block(fd_t * fdt);
void proc_unblock(int pid);
int proc_get_flag(int flag);
void proc_set_flag(int flag);
void proc_clear_flag(int flag);

#endif
