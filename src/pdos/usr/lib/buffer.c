#include "stdlib.h"
#include "sys.h"

typedef struct {
    char * buf;
    unsigned int fd;
    unsigned int pos;
    unsigned int buflen;
} buf_t;

void buffer_init(buf_t * buf, int fd, char * buffer, int buflen) {
    buf->buf = buffer;
    buf->fd = fd;
    buf->pos = 0;
    buf->buflen = buflen;
}

void buffer_put(buf_t * buf, char c) {
    buf->buf[buf->pos] = c;
    buf->pos++;
    if (buf->pos == buf->buflen) {
        write(buf->fd, buf->buf, buf->buflen);
        buf->pos = 0;
    }
}

void buffer_write(buf_t * buf, char * str) {
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        buffer_put(buf, str[i]);
    }
}

void buffer_flush(buf_t * buf) {
    if (buf->pos > 0) {
        write(buf->fd, buf->buf, buf->pos);
        buf->pos = 0;
    }
}


