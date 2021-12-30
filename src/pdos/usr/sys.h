#ifndef SYS_H
#define SYS_H

void exit(int status);
int read(int n, char * buf);
int write(int n, char * buf);
void flush();
int fopen(char * path, char mode);
int fclose(int fd);
int fseek(int fd, unsigned int pos);
int fread(int fd, unsigned char * buf, unsigned int len);
int fwrite(int fd, unsigned char * buf, int len);
int fflush(int fd);
int link(char * src, char * dst);
int unlink(char * target);
int mkdir(char * dirname);
int rmdir(char * dirname);

#endif
