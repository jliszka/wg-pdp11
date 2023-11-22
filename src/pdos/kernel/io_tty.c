#include "proc.h"
#include "io.h"
#include "tty.h"

int io_tty_read(int fd, unsigned char * buf, unsigned int len) {
    if (proc_get_flag(PROC_FLAG_HAS_TTY_IN) == 0) {
        // Received EOF
        return 0;
    }

    char ch;
    fd_t * fdt = proc_fd(fd);
    int tty = fdt->cur_block; // a hack; this is the tty number
    int bytes_read = tty_read(tty, len-2, buf, &ch);

    if (ch == CTRL_D) {
        // Set a flag indicating EOF
        proc_clear_flag(PROC_FLAG_HAS_TTY_IN);
    } else {
        buf[bytes_read++] = ch;
    }
    buf[bytes_read++] = 0;

    return bytes_read;
}

int io_tty_write(int fd, unsigned char * buf, unsigned int len) {
    fd_t * fdt = proc_fd(fd);
    int tty = fdt->cur_block;
    return tty_write(tty, len, buf);
}

int io_tty_flush(int fd) {
    fd_t * fdt = proc_fd(fd);
    int tty = fdt->cur_block; // a hack; this is the tty number
    tty_flush(tty);
    return 0;
}
