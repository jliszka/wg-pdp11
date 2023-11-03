#ifndef IO_HD_H
#define IO_HD_H

int io_hd_seek(int fd, unsigned int pos);
int io_hd_read(int fd, unsigned char * buf, unsigned int len);
int io_hd_write(int fd, unsigned char * buf, unsigned int len);
int io_hd_flush(int fd);

#endif
