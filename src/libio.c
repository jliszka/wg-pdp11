#define KBV 060     // keyboard (TTI) interrupt vector
#define KBS 0177560 // keyboard status register
#define KBD 0177562 // keyboard data buffer
#define KBR 0x80    // keyboard ready bit

#define TTV 064     // terminal (TTO) interrupt vector
#define TTS 0177564 // terminal status register
#define TTD 0177566 // terminal data buffer
#define TTR 0x80    // terminal ready bit

char readchr()
{
    volatile unsigned int *xcsr = (unsigned int *)KBS;
    unsigned char *xbuf = (unsigned char *)KBD;
    while (!(*xcsr & KBR))
        ;
    char c = *xbuf;
    return c;
}

void writechr(char c)
{
    volatile unsigned int *xcsr = (unsigned int *)TTS;
    unsigned char *xbuf = (unsigned char *)TTD;
    while (!(*xcsr & TTR))
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
