#ifndef SYS_H
#define SYS_H

#include "fs.h"

void exit(int status);
void halt();
int fork();
int exec(int argc, char * argv[]);
int open(char * path, char mode);
int close(int fd);
int lseek(int fd, unsigned int pos);
int read(int fd, unsigned char * buf, unsigned int len);
int write(int fd, unsigned char * buf, unsigned int len);
int fsync(int fd);
int link(char * src, char * dst);
int unlink(char * target);
int mkdir(char * dirname);
int rmdir(char * dirname);
int stat(char * path, stat_t * stat);
int mkfs();
int wait(int pid);
int chdir(const char * path);
int getcwd(char * buf, unsigned int len);
int pipe(int * writefd, int * readfd);
int dup2(int oldfd, int newfd);

#endif
