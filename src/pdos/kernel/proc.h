#ifndef PROC_H
#define PROC_H

#include "io.h"

#define PROC_FLAG_HAS_TTY_IN 1
#define PROC_FLAG_HAS_TTY_OUT 2

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
void proc_unblock(int pid);
int proc_get_flag(int flag);
void proc_set_flag(int flag);
void proc_clear_flag(int flag);

#endif
