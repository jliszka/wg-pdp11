#include "stdlib.h"
#include "sys.h"

void print(char * str) {
    fprint(STDOUT, str);
}

void println(char * str) {
    fprintln(STDOUT, str);
}

void fprint(int fd, char * str) {
    int len = strlen(str);
	int written = 0;
	do {
		written += write(fd, str + written, len - written);
	} while (written < len);
}

void fprintln(int fd, char * str) {
    fprint(fd, str);
    fprint(fd, "\r\n");
    fsync(fd);
}

int input(int len, char * buf) {
	return read(STDIN, buf, len);
}
