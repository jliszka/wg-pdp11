#ifndef FS_H
#define FS_H

#define STDIN 0
#define STDOUT 1

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
int is_dir(stat_t * stat);

#define INODE_FLAG_INDIRECT 1
#define INODE_FLAG_DIRECTORY 2
#define INODE_FLAG_CHAR_DEVICE 0x10
#define INODE_FLAG_BLOCK_DEVICE 0x20

#endif
