
#include "libio.h"

void crlf()
{
    writechr(CR);
    writechr(LF);
}

void hello()
{
    crlf();
    crlf();
    writestr("hello");
    crlf();
    writestr("w0rld");
    crlf();
    crlf();
}

void echo()
{
    writestr("Type . to quit\r\n");
    writechr('>');
    writechr(' ');
    while (1)
    {
        char c = readchr();
        writechr(c);
        if (c == CR)
        {
            writechr(LF);
            writechr('>');
            writechr(' ');
        }

        if (c == '.')
        {
            writestr("\r\n\ngoodbye\r\n");
            return;
        }
    }
}

int main()
{
    hello();
    echo();
}
