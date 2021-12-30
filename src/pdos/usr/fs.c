#include "fs.h"
#include "sys.h"

int opendir(char * dirname) {
    return fopen(dirname, 'd');
}

int readdir(int fd, dirent_t * dirent) {
    return fread(fd, (unsigned char *)dirent, sizeof(dirent_t));
}

int closedir(int fd) {
    return fclose(fd);
}
