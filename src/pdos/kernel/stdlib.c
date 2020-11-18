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
    return 0;
}

void strncpy(char * dst, char * src, int n)
{
    // Copy up to n-1 characters and null-terminate
    while (*src && --n)
    {
        *dst++ = *src++;
    }
    *dst = 0;
}

int strntok(char * str, char delim, char * tokens[], int ntokens)
{
    int token_count;
    for (token_count = 0; token_count < ntokens; token_count++) {
        // Find the beginnin of the token.
        while (*str == delim && *str != 0) str++;
        // End of the string! Return.
        if (*str == 0) return token_count;
        // Found the beginning of a token.
        tokens[token_count] = str;

        // Find the end of the token.
        while (*str != delim && *str != 0) str++;
        // End of the string! Return.
        if (*str == 0) return token_count+1;
        // Found the end of the token. Replace the delimiter with a \0.
        *str++ = 0;
    }
    return token_count;
}