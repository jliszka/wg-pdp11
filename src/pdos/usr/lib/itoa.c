#include "stdlib.h"
#include "sys.h"

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
