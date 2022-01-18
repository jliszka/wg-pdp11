#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {

    int ptr = fopen("/dev/ptr", 'r');
    if (ptr < 0) {
        println("Unable to open PTR device");
        return ptr;
    }
    
    for (int i = 1; i < argc; i++) {
        print("Attach ");
        print(argv[i]);
        println("...");
        halt();
        
        int fd = fopen(argv[i], 'w');
        if (fd < 0) {
            print("Unable to write ");
            println(argv[i]);
            return fd;
        }

        unsigned char buf[64];
        int in = fread(ptr, buf, 64);
        int out = 0;
        int total = 0;
        while (in > 0) {
            out = fwrite(fd, buf, in);
            if (out < 0) return out;
            while (out < in) {
                int ret = fwrite(fd, buf+out, in-out);
                if (ret < 0) return ret;
                out += ret;
            }
            total += out;
            in = fread(ptr, buf, 64);
        }
        fclose(fd);
        if (in < 0) {
            return in;
        }

        print(itoa(10, total, buf));
        println(" bytes copied.");
    }

    fclose(ptr);

    return 0;
}
