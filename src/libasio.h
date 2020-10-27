#ifndef LIBASIO_H
#define LIBASIO_H

void io_init();
void read(char * dst);
void write(char * str);
void flush();
void kb_handler();
void tt_handler();

#endif
