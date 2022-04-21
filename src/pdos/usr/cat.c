#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
    for (int i = 1; i < argc; i++) {
        int fd = fopen(argv[i], 'r');
        if (fd < 0) {
            print("Failed to open ");
            println(argv[i]);
            return fd;
        }

        unsigned char buf[64];
        int in = fread(fd, buf, 64);
        while (in > 0) {
            fwrite(STDOUT, buf, in);
            in = fread(fd, buf, 64);
        }
        fflush(STDOUT);
        if (in < 0) {
            return in;
        }
        fclose(fd);
    }

    return 0;
}
