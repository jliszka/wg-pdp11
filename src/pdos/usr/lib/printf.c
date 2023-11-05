#include <stdarg.h>
#include "stdlib.h"
#include "sys.h"
#include "buffer.h"

void _format_number(buf_t * buf, int radix, int n, int width, char pad) {
    char num[16];
    itoa(radix, n, num);
    for (int i = strlen(num); i < width; i++) {
        buffer_put(buf, pad);
    }
    buffer_write(buf, num);    
}

void vfprintf(int fd, const char * fmt, va_list args) {
    char buffer[32];
    buf_t buf;
    buffer_init(&buf, fd, buffer, 32);

    for (const char * p = fmt; *p; p++) {
        if (*p != '%') {
            buffer_put(&buf, *p);
            continue;
        }
        p++;

        if (*p == '%') {
            buffer_put(&buf, '%');
            continue;
        }

        char pad = ' ';
        if (*p == '0') {
            pad = '0';
            p++;
        }

        int width = 0;
        while ('0' <= *p && *p <= '9') {
            width *= 10;
            width += *p - '0';
            p++;
        }

        char * s;
        char c;
        int n;
        switch (*p) {
        case 'd':
            n = va_arg(args, int);
            _format_number(&buf, 10, n, width, pad);
            break;
        case 'x':
            n = va_arg(args, int);
            _format_number(&buf, 16, n, width, pad);
            break;
        case 'o':
            n = va_arg(args, int);
            _format_number(&buf, 8, n, width, pad);
            break;
        case 'b':
            n = va_arg(args, int);
            _format_number(&buf, 2, n, width, pad);
            break;
        case 's':
            s = va_arg(args, char *);
            for (int i = strlen(s); i < width; i++) {
                buffer_put(&buf, ' ');
            }            
            buffer_write(&buf, s);
            break;
        case 'c':
            c = va_arg(args, int);
            for (int i = 1; i < width; i++) {
                buffer_put(&buf, ' ');
            }
            buffer_put(&buf, c);
            break;
        default:
            buffer_put(&buf, *p);
        }
    }
    buffer_flush(&buf);
}

void printf(const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(STDOUT, fmt, args);
    va_end(args);    
}

void fprintf(int fd, const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(fd, fmt, args);
    va_end(args);
}
