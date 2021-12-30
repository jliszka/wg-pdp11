#include "stdlib.h"
#include "sys.h"
#include "fs.h"

int main(int argc, char ** argv) {
    char * dirname = ".";
    if (argc > 1) {
        dirname = argv[1];
    }
    int fd = opendir(dirname);

    dirent_t dir;
    unsigned char buf[16];
    while (readdir(fd, &dir)) {
        print(itoa(10, dir.inode, buf));
        print(" ");
        println(dir.filename);
    }

    closedir(fd);

    return 0;
}
