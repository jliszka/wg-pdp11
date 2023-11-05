#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {

    int ptr = open("/dev/ptr", 'r');
    if (ptr < 0) {
        println("Unable to open PTR device");
        return ptr;
    }
    
    for (int i = 1; i < argc; i++) {
        printf("Attach %s...\r\n", argv[i]);
        halt();
        
        int fd = open(argv[i], 'w');
        if (fd < 0) {
            printf("Unable to write %s\r\n", argv[i]);
            return fd;
        }

        unsigned char buf[64];
        int in = read(ptr, buf, 64);
        int out = 0;
        int total = 0;
        while (in > 0) {
            out = write(fd, buf, in);
            if (out < 0) return out;
            while (out < in) {
                int ret = write(fd, buf+out, in-out);
                if (ret < 0) return ret;
                out += ret;
            }
            total += out;
            in = read(ptr, buf, 64);
        }
        close(fd);
        if (in < 0) {
            return in;
        }

        printf("%d bytes copied.\r\n", total);
    }

    close(ptr);

    return 0;
}
