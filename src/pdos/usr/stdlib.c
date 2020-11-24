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

