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

char * uitoa(int radix, int n, char * dst) {
    if (n == 0) {
        dst[0] = '0';
        dst[1] = 0;
        return dst;
    }

    // The only way to do this is to do it backwards
    // (least significant digit first)
    char * p = dst;
    while (n > 0) {
        int k = n % radix;
        *p++ = k > 9 ? 'a' + (k-10) : '0' + k;
        n = n / radix;
    }
    *p = 0;

    // And then reverse the string
    char * q = dst;
    while (q < p) {
        char t = *--p;
        *p = *q;
        *q++ = t;
    }

    return dst;
}

char * itoa(int radix, int n, char * dst) {
    if (n < 0) {
        *dst = '-';
        uitoa(radix, -n, dst+1);
        return dst;
    }
    return uitoa(radix, n, dst);
}

int atoi(char * str) {
    int ret = 0;
    int sign = 1;
    if (*str == '0') {
        return 0;
    }
    if (*str == '-') {
        sign = -1;
        str++;
    }
    while ('0' <= *str && *str <= '9') {
        ret *= 10;
        ret += *str++ - '0';
    }
    return ret * sign;
}

int strncmp(char * a, char * b, int n)
{
    // Compare the first n characters and the \0
    while (*a && *b && n--)
    {
        char diff = *b++ - *a++;
        if (diff != 0)
        {
            return diff;
        }
    }
    return *b - *a;
}

char * strncpy(char * dst, char * src, int n)
{
    // Copy up to n-1 characters and null-terminate
    while (*src && --n)
    {
        *dst++ = *src++;
    }
    *dst++ = 0;
    return dst;
}


