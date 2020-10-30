#ifndef LIBASIO_H
#define LIBASIO_H

void io_init();
void read(char * dst);
void write(char * str);
void writeln(char * str);
void flush();
void kb_handler();
void tt_handler();

int ptr_has_next();
unsigned char ptr_next();
int ptr_read(int bytes, unsigned char * dst);

#endif
