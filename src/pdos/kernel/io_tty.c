#include "tty.h"

int io_tty_read(int fd, unsigned char * buf, unsigned int len) {
    return tty_read(len, buf);
}

int io_tty_write(int fd, unsigned char * buf, unsigned int len) {
    return tty_write(len, buf);
}

int io_tty_flush(int fd) {
    tty_flush();
    return 0;
}
