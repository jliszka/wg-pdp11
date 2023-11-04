#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {

    int lines = 10;

    char opt;
    char *optarg;
    while ((opt = getopt(&argc, &argv, "n:", &optarg)) != -1) {
        switch (opt) {
            case 'n':
                lines = atoi(optarg, strlen(optarg));
                break;
        }
    }

    int fd = STDIN;
    if (argc == 2) {
        fd = open(argv[1], 'r');
        if (fd < 0) {
            return fd;
        }
    }

    char buf[256];
    buf[255] = 0;
    while (lines > 0) {
        int ret = read(fd, buf, 255);
        if (ret == 0) {
            break;
        }
        if (ret < 0) {
            return ret;
        }
        for (int i = 0; i < ret; i++) {
            if (buf[i] == '\n') {
                lines--;
            }
            if (lines == 0) {
                buf[i+1] = 0;
                break;
            }
        }

        print(buf);
    }

    return 0;
}
