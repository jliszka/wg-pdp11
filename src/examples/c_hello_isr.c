
#include "libasio.h"

unsigned char buf[64];

int main()
{
    io_init();
    write("\r\nHello! What is your name?\r\n");
    flush();
    read(buf);
    write("\r\nHi, ");
    write(buf);
    write(", nice to meet you :)\r\n");
    flush();
}
