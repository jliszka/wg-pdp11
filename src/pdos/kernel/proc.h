#ifndef PROC_H
#define PROC_H

#include "io.h"

fd_t * proc_fd_alloc(int * fd);
fd_t * proc_fd(int fd);
int proc_create();
int proc_dup(int pid);
void proc_free(int pid);
int proc_exec(int argc, char ** argv);

#endif
