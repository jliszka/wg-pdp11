#ifndef IO_H
#define IO_H

#include "fs_defs.h"

typedef struct {
    int (*lseek)(int, unsigned int);
    int (*read)(int, unsigned char *, unsigned int);
    int (*write)(int, unsigned char *, unsigned int);
    int (*fsync)(int);
} vfile_t;

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
} fd_t;

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
