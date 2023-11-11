#include <stdlib.h>
#include <sys.h>


int write_out_file(int fd) {
    unsigned char buf[64];
    int in = read(fd, buf, 64);
    while (in > 0) {
        write(STDOUT, buf, in);
        in = read(fd, buf, 64);
    }
    fsync(STDOUT);
    return in;
}

int main(int argc, char ** argv) {
    if (argc == 1) {
        // No args, read STDIN
        return write_out_file(STDIN);
    }
    for (int i = 1; i < argc; i++) {
        int fd;
        if (strncmp(argv[i], "-", 2) == 0) {
            fd = STDIN;
        } else {
            fd = open(argv[i], 'r');
            if (fd < 0) {
                print("Failed to open ");
                println(argv[i]);
                return fd;
            }
        }

        int ret = write_out_file(fd);
        if (ret < 0) {
            printf("Error reading %s: %d\r\n", argv[i], ret);
            return ret;
        }

        close(fd);
    }

    return 0;
}
