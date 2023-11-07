#include <stdlib.h>
#include <sys.h>
#include <errno.h>

int main(int argc, char ** argv) {
    if (argc < 3) {
        println("Not enough args.");
        return -1;
    }

    stat_t file_stat;
    int ret = stat(argv[argc-1], &file_stat);
    if (
        argc > 3 &&
        (ret == ERR_FILE_NOT_FOUND || !is_dir(&file_stat))) {
        printf("Last arg must be a directory");
        return -1;
    }

    for (int i = 1; i < argc-1; i++) {
        ret = rename(argv[i], argv[argc-1]);
        if (ret < 0) return ret;
    }
    return 0;
}
