#include "stdio.h"
#include "stdlib.h"
#include "tty.h"

void print(char * str) {
    int len = strlen(str)+1;
    int written = 0;
    do {
        written += tty_write(0, len - written, str + written);
    } while (written < len);
}

void println(char * str) {
    print(str);
    print("\r\n");
    tty_flush(0);
}
