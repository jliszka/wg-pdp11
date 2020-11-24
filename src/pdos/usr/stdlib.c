#include "stdlib.h"
#include "sys.h"

int strlen(char * str) {
	int len = 0;
	while (*str++) len++;
	return len;
}

void print(char * str) {
	int len = strlen(str)+1;
	int written = 0;
	do {
		written += write(len - written, str + written);
	} while (written < len);
}

void println(char * str) {
	print(str);
	write(3, "\r\n");
	flush();
}

int input(int len, char * buf) {
	int c = read(len, buf);
	if (c < 2) return c;
	// byte count includes null terminator
	while (buf[c-2] != '\r') {
		c += read(len-c-1, buf+c-1);
	}
	return c;
}
