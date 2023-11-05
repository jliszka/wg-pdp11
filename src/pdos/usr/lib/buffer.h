#ifndef BUFFER_H
#define BUFFER_H

typedef struct {
    char * buf;
    unsigned int fd;
    unsigned int pos;
    unsigned int buflen;
} buf_t;

void buffer_init(buf_t * buf, int fd, char * buffer, int buflen);
void buffer_put(buf_t * buf, char c);
void buffer_write(buf_t * buf, char * str);
void buffer_flush(buf_t * buf);

#endif
