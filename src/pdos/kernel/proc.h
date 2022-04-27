#ifndef PROC_H
#define PROC_H

#include "io.h"

int proc_fd_alloc(fd_t ** fdt);
void proc_fd_free(int fd);
fd_t * proc_fd(int fd);
int proc_create();
int proc_dup(int pid);
void proc_free(int pid);
int proc_exec(int argc, char ** argv);

#endif
