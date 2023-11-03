#ifndef SYS_H
#define SYS_H

#include "fs.h"

void exit(int status);
void halt();
int fork();
int exec(int argc, char * argv[]);
int fopen(char * path, char mode);
int fclose(int fd);
int fseek(int fd, unsigned int pos);
int fread(int fd, unsigned char * buf, unsigned int len);
int fwrite(int fd, unsigned char * buf, unsigned int len);
int fflush(int fd);
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

#endif
