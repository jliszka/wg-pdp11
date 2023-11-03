#ifndef IO_FILE_H
#define IO_FILE_H

int io_file_seek(int fd, unsigned int pos);
int io_file_read(int fd, unsigned char * buf, unsigned int len);
int io_file_write(int fd, unsigned char * buf, unsigned int len);
int io_file_flush(int fd);

#endif
