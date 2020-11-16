#include "stdlib.h"

char * itoa(int radix, int n, char * dst) {
    if (n < 0) {
        *dst = '-';
        itoa(radix, -n, dst+1);
        return dst;
    }

    // The only way to do this is to do it backwards
    // (least significant digit first)
    char * p = dst;
    while (n > 0) {
        *p++ = '0' + (n % radix);
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
