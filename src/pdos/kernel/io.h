#ifndef IO_H
#define IO_H

int io_reset();
int io_fopen(char * path, char mode);
int io_fclose(int fd);
int io_fseek(int fd, unsigned int pos);
int io_fread(int fd, unsigned char * buf, unsigned int len);
int io_fwrite(int fd, unsigned char * buf, unsigned int len);
int io_fflush(int fd);

#endif
