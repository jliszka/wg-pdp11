#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], 'r');
        if (fd < 0) {
            print("Failed to open ");
            println(argv[i]);
            return fd;
        }

        unsigned char buf[64];
        int in = read(fd, buf, 64);
        while (in > 0) {
            write(STDOUT, buf, in);
            in = read(fd, buf, 64);
        }
        fsync(STDOUT);
        if (in < 0) {
            return in;
        }
        close(fd);
    }

    return 0;
}
