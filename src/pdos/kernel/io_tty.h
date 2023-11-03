#ifndef IO_TTY_H
#define IO_TTY_H

int io_tty_read(int fd, unsigned char * buf, unsigned int len);
int io_tty_write(int fd, unsigned char * buf, unsigned int len);
int io_tty_flush(int fd);

#endif
