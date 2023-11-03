#ifndef PROC_H
#define PROC_H

#include "io.h"

void proc_init();
int proc_fd_alloc(fd_t ** fdt);
void proc_fd_free(int fd, int pid);
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

#endif
