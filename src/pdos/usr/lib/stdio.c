#include "stdlib.h"
#include "sys.h"

void print(char * str) {
    fprint(STDOUT, str);
}

void println(char * str) {
    fprintln(STDOUT, str);
}

void fprint(int fd, char * str) {
    int len = strlen(str)+1;
	int written = 0;
	do {
		written += fwrite(fd, str + written, len - written);
	} while (written < len);
}

void fprintln(int fd, char * str) {
    fprint(fd, str);
    fprint(fd, "\r\n");
    fflush(fd);
}

int input(int len, char * buf) {
	return fread(STDIN, buf, len);
}
