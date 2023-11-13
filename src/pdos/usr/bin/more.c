#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {

    int lines = 19;

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

    int stdin = open("/dev/tty", 'r');
    if (stdin < 0) {
        return stdin;
    }

    char buf[256];
    buf[255] = 0;
    int pos = 0;
    int line = 0;
    while (1) {
        int ret = read(fd, buf, 255);
        if (ret == 0) {
            break;
        }
        if (ret < 0) {
            return ret;
        }
        for (int i = 0; i < ret; i++) {
            if (buf[i] == '\n') {
                line++;
                char c = buf[i+1];
                buf[i+1] = 0;
                print(buf + pos);
                buf[i+1] = c;
                pos = i+1;
            }
            if (line == lines) {
                print("-- Press ENTER --");
                fsync(STDOUT);
                read(stdin, buf, 2);
                if (strncmp(buf, "q", 2) == 0) {
                    break;
                }
                line = 0;
            }
        }
    }

    return 0;
}
