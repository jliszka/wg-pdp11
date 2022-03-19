#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
    for (int i = 1; i < argc; i++) {
        int fd = fopen(argv[i], 'r');
        if (fd < 0) {
            println("Failed to open source");
            return fd;
        }

        unsigned char buf[64];
        int in = fread(fd, buf, 64);
        while (in > 0) {
            print(buf);
            in = fread(fd, buf, 64);
        }
        if (in < 0) {
            return in;
        }
        fclose(fd);
    }

    return 0;
}
