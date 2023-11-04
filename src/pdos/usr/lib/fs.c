#include "fs.h"
#include "sys.h"

int opendir(char * dirname) {
    return open(dirname, 'd');
}

int readdir(int fd, dirent_t * dirent) {
    return read(fd, (unsigned char *)dirent, sizeof(dirent_t));
}

int closedir(int fd) {
    return close(fd);
}

int is_dir(stat_t * file_stat) {
    if (file_stat->flags == INODE_FLAG_DIRECTORY) {
        return 1;
    }
    return 0;
}
