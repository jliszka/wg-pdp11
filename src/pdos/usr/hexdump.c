#include "stdlib.h"
#include "sys.h"

int main(int argc, char ** argv) {
    if (argc < 2) {
        return 0;
    }
    int radix = 8;
    int idx = 1;
    if (argv[1][0] == '-') {
        idx = 2;
        switch (argv[1][1]) {
        case 'o':
            radix = 8;
            break;
        case 'd':
            radix = 10;
            break;
        case 'h':
            radix = 16;
            break;
        case 'b':
            radix = 2;
            break;
        }
    }
        
    for (int i = idx; i < argc; i++) {
        int fd = fopen(argv[i], 'r');
        if (fd < 0) {
            println("Failed to open source");
            return fd;
        }

        unsigned char inbuf[8];
        unsigned char outbuf[16];
        int pos = 0;
        int n = fread(fd, inbuf, 8);
        while (n > 0) {
            print(itoa(16, pos, outbuf));
            print(" ");
            for (int j = 0; j < n; j++) {
                print(uitoa(radix, inbuf[j], outbuf));
                print(" ");
            }
            pos += n;
            n = fread(fd, inbuf, 64);
            println("");
        }
        fclose(fd);
    }

    return 0;
}
