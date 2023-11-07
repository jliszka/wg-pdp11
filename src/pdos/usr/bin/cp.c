#include <stdlib.h>
#include <sys.h>
#include <fs.h>
#include <errno.h>

int do_copy(char * src_path, char * dst_path);

int main(int argc, char ** argv) {
    if (argc < 3) {
        println("Not enough args.");
        return -1;
    }

    stat_t file_stat;
    int ret = stat(argv[argc-1], &file_stat);
    if (
        argc == 3 &&
        (ret == ERR_FILE_NOT_FOUND || !is_dir(&file_stat))) {
        // 2-arg file to file copy
        return do_copy(argv[1], argv[2]);
    }

    // File to directory copy
    if (!is_dir(&file_stat)) {
        printf("Last arg must be a directory");
        return -1;
    }

    // Prepare dst directory path with ending /
    char dst_path[32];
    char * dst_tail = strncpy(dst_path, argv[argc-1], 32);
    int len = dst_tail - dst_path;
    if (dst_path[len-1] != '/') {
        dst_tail = strncpy(dst_tail, "/", 32-len);
        len += 1;
    }

    for (int i = 1; i < argc-1; i++) {
        // Find filename part of src path
        char * filename = argv[i];
        for (char * p = filename; *p; *p++) {
            if (*p == '/') {
                filename = p+1;
            }
        }
        // Construct src path
        strncpy(dst_tail, filename, 32-len);
        int ret = do_copy(argv[i], dst_path);
        if (ret < 0) {
            return ret;
        }
    }

    return 0;
}

int do_copy(char * src_path, char * dst_path) {

    int src = open(src_path, 'r');
    if (src < 0) {
        printf("Failed to open source %s\r\n", src_path);
        return src;
    }

    int dst = open(dst_path, 'w');
    if (dst < 0) {
        printf("Failed to open target %s\r\n", dst_path);
        return dst;
    }

    unsigned char buf[64];
    int in = read(src, buf, 64);
    while (in > 0) {
        int out = write(dst, buf, in);
        if (out < 0) {
            printf("Failed while writing %s\r\n", dst_path);
            return out;
        }
        while (out < in) {
            int ret = write(dst, buf+out, in-out);
            if (ret < 0) {
                return ret;
                printf("Failed while writing %s\r\n", dst_path);
            }
            out += ret;
        }
        in = read(src, buf, 64);
    }
    if (in < 0) {
        printf("Failed while reading %s\r\n", src_path);
        return in;
    }

    close(dst);
    close(src);

    return 0;
}
