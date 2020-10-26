
#include "libio.h"

void hello()
{
    writechr(CR);
    writechr(LF);
    writechr(CR);
    writechr(LF);
    writestr("hello");
    writechr(CR);
    writechr(LF);
    writestr("w0rld");
    writechr(CR);
    writechr(LF);
    writechr(CR);
    writechr(LF);
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
