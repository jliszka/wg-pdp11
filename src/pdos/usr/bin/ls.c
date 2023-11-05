#include <stdlib.h>
#include <sys.h>
#include <fs.h>

int main(int argc, char ** argv) {
    int list_long = 0;
    char opt;
    char *optarg;
    while ((opt = getopt(&argc, &argv, "l", &optarg)) != -1) {
        switch (opt) {
            case 'l':
                list_long = 1;
                break;
        }
    }

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

        if (list_long) {
            printf("%2d %d %4d ",
                   dir.inode,
                   file_stat.refcount,
                   file_stat.filesize);
        }
        printf("%s%s\r\n",
               dir.filename,
               is_dir(&file_stat) ? "/" : "");
    }

    closedir(dd);

    return 0;
}
