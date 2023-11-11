#ifndef LIBASIO_H
#define LIBASIO_H

#define CTRL_C 3
#define CTRL_D 4
#define CTRL_Z 26

void tty_init();
int tty_read(int nbytes, char * dst, char * endch);
int tty_write(int nbytes, char * str);
unsigned char tty_getch();
void tty_flush();
void kb_handler();
void tt_handler();

#endif
