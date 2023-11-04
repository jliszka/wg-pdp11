#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {

    int ptr = open("/dev/ptr", 'r');
    if (ptr < 0) {
        println("Unable to open PTR device");
        return ptr;
    }
    
    for (int i = 1; i < argc; i++) {
        print("Attach ");
        print(argv[i]);
        println("...");
        halt();
        
        int fd = open(argv[i], 'w');
        if (fd < 0) {
            print("Unable to write ");
            println(argv[i]);
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

        print(itoa(10, total, buf));
        println(" bytes copied.");
    }

    close(ptr);

    return 0;
}
