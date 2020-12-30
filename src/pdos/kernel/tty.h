#ifndef LIBASIO_H
#define LIBASIO_H

void tty_init();
int tty_read(int nbytes, char * dst);
int tty_write(int nbytes, char * str);
unsigned char tty_getch();
void tty_flush();
void kb_handler();
void tt_handler();

#endif
