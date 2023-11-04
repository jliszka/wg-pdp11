#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
    if (argc != 3) {
        println("Wrong number of args.");
        return -1;
    }

    int src = open(argv[1], 'r');
    if (src < 0) {
        println("Failed to open source");
        return src;
    }
    int dst = open(argv[2], 'w');
    if (dst < 0) {
        println("Failed to open target");
        return dst;
    }

    unsigned char buf[64];
    int in = read(src, buf, 64);
    while (in > 0) {
        int out = write(dst, buf, in);
        if (out < 0) return out;
        while (out < in) {
            int ret = write(dst, buf+out, in-out);
            if (ret < 0) return ret;
            out += ret;
        }
        in = read(src, buf, 64);
    }
    if (in < 0) {
        return in;
    }

    close(dst);
    close(src);

    return 0;
}
