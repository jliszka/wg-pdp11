#include <stdlib.h>
#include <sys.h>
#include <fs.h>

int main(int argc, char ** argv) {
    char * dirname = ".";
    if (argc > 1) {
        dirname = argv[1];
    }
    int dd = opendir(dirname);
    if (dd < 0) {
        print("Could not read dir ");
        println(dirname);
        return dd;
    }

    dirent_t dir;
    unsigned char buf[16];
    char path[64];
    char * tail = strncpy(path, dirname, 64);
    tail = strncpy(tail, "/", 2);
    int len = tail - path;

    while (readdir(dd, &dir)) {
        strncpy(tail, dir.filename, 64-len);
        stat_t file_stat;
        int ret = stat(path, &file_stat);
        if (ret < 0) {
            print("Failed to stat path ");
            println(path);
            return ret;
        }

        print(itoa(10, dir.inode, buf));
        print(" ");
        print(itoa(10, file_stat.filesize, buf));
        print(" ");
        print(dir.filename);
        if (is_dir(&file_stat)) {
            print("/");
        }
        println("");
    }

    closedir(dd);

    return 0;
}
