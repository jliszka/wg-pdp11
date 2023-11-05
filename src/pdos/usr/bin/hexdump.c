#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
    if (argc < 2) {
        return 0;
    }
    int radix = 16;
    char * fmt = "%02x ";

    char opt;
    char *optarg;
    while ((opt = getopt(&argc, &argv, "odbh", &optarg)) != -1) {
        switch (opt) {
        case 'o':
            radix = 8;
            fmt = "%03o ";
            break;
        case 'd':
            radix = 10;
            fmt = "%d ";
            break;
        case 'b':
            radix = 2;
            fmt = "%08b ";
            break;
        case 'h':
            radix = 16;
            fmt = "%02x ";
            break;
        }
    }
        
    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], 'r');
        if (fd < 0) {
            println("Failed to open source");
            return fd;
        }

        unsigned char inbuf[8];
        int pos = 0;
        int n = read(fd, inbuf, 8);
        while (n > 0) {
            printf("%x: ", pos);
            for (int j = 0; j < n; j++) {
                printf(fmt, inbuf[j]);
            }
            pos += n;
            n = read(fd, inbuf, 8);
            println("");
        }
        close(fd);
    }

    return 0;
}
