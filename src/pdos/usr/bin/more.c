#include <stdlib.h>
#include <sys.h>
#include <buffer.h>

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

    char inbuf[256];
    int line = 0;

    char buffer[256];
    buf_t buf;
    buffer_init(&buf, STDOUT, buffer, 256);

    while (1) {
        int ret = read(fd, inbuf, 255);
        if (ret == 0) {
            break;
        }
        if (ret < 0) {
            return ret;
        }
        for (int i = 0; i < ret; i++) {
            buffer_put(&buf, inbuf[i]);
            if (inbuf[i] == '\n') {
                buffer_flush(&buf);
                fsync(STDOUT);
                line++;
                if (line == lines) {
                    print("-- Press ENTER --");
                    fsync(STDOUT);
                    read(stdin, inbuf, 2);
                    if (strncmp(inbuf, "q", 2) == 0) {
                        break;
                    }
                    line = 0;
                }
            }
        }
    }
    buffer_flush(&buf);
    fsync(STDOUT);

    return 0;
}
