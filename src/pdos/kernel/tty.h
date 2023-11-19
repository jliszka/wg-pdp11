#ifndef LIBASIO_H
#define LIBASIO_H

#define CTRL_C 3
#define CTRL_D 4
#define CTRL_Z 26

void tty_init();
int tty_read(int tty, int nbytes, char * dst, char * endch);
int tty_write(int tty, int nbytes, char * str);
unsigned char tty_getch(int tty);
void tty_flush(int tty);
void kb_handler(int tty);
void tt_handler(int tty);

#endif
