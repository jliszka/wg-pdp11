#ifndef IO_H
#define IO_H

typedef struct {
    int (*fseek)(int, unsigned int);
    int (*fread)(int, unsigned char *, unsigned int);
    int (*fwrite)(int, unsigned char *, unsigned int);
    int (*fflush)(int);
} vfile_t;

typedef struct {
    int inode;
    int cur_block;
    int pos;
    int max_pos;
    char mode;
    vfile_t * vfile;
    unsigned char * buffer;
} fd_t;

int io_reset();
int io_fopen(char * path, char mode);
int io_fclose(int fd);
int io_fseek(int fd, unsigned int pos);
int io_fread(int fd, unsigned char * buf, unsigned int len);
int io_fwrite(int fd, unsigned char * buf, unsigned int len);
int io_fflush(int fd);

#endif
