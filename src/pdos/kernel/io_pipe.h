#ifndef IO_PIPE_H
#define IO_PIPE_H

int io_pipe_read(int fd, unsigned char * buf, unsigned int len);
int io_pipe_write(int fd, unsigned char * buf, unsigned int len);
int io_pipe_flush(int fd);

#endif
