#ifndef IO_H
#define IO_H

#include "fs_defs.h"

typedef struct {
    int (*lseek)(int, unsigned int);
    int (*read)(int, unsigned char *, unsigned int);
    int (*write)(int, unsigned char *, unsigned int);
    int (*fsync)(int);
} vfile_t;

#include "proc.h"

int io_reset();
int io_open(char * path, char mode);
int io_close(int fd);
int io_lseek(int fd, unsigned int pos);
int io_read(int fd, unsigned char * buf, unsigned int len);
int io_write(int fd, unsigned char * buf, unsigned int len);
int io_fsync(int fd);
int io_stat(int fd, stat_t * stat);
int io_dup2(int oldf, int newfd);

#endif
