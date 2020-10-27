#include "libio.h"

#define KBV 060     // keyboard (TTI) interrupt vector
#define KBS 0177560 // keyboard status register
#define KBD 0177562 // keyboard data buffer
#define KB_READY 0x80    // keyboard ready bit
#define KB_ENABLE 0x40    // keyboard interrupt enable bit

#define TTV 064     // terminal (TTO) interrupt vector
#define TTS 0177564 // terminal status register
#define TTD 0177566 // terminal data buffer
#define TT_READY 0x80    // terminal ready bit
#define TT_ENABLE 0x40    // terminal interrupt enable bit

char readchr()
{
    volatile unsigned int *xcsr = (unsigned int *)KBS;
    unsigned char *xbuf = (unsigned char *)KBD;
    while (!(*xcsr & KB_READY))
        ;
    char c = *xbuf;
    return c;
}

void writechr(char c)
{
    volatile unsigned int *xcsr = (unsigned int *)TTS;
    unsigned char *xbuf = (unsigned char *)TTD;
    while (!(*xcsr & TT_READY))
        ;
    *xbuf = c;
}

void writestr(char *s)
{
    while (*s != 0)
    {
        writechr(*s++);
    }
}

