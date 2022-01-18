#ifndef FS_H
#define FS_H

typedef struct {
    unsigned int inode;
    char filename[14];
} dirent_t;

typedef struct {
    unsigned int inode;
    unsigned int filesize;
    unsigned char refcount;
    unsigned char flags;
} stat_t;

int opendir(char * dirname);
int readdir(int fd, dirent_t * dirent);
int closedir(int fd);

#endif
