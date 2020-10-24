#define kbv 60     // keyboard (TTI) interrupt vector
#define kbs 177560 // keyboard status register
#define kbd 177562 // keyboard data buffer

#define TTV 64     // terminal (TTO) interrupt vector
#define TTS 177564 // terminal status register
#define TTD 177566 // terminal data buffer
#define TTR 0x80   // terminal ready bit

#define CR 15
#define LF 12

void write(char c)
{
    volatile unsigned int *xcsr = (unsigned int *)TTS;
    unsigned char *xbuf = (unsigned char *)TTD;
    while (!(*xcsr & TTR))
        ;
    *xbuf = c;
}

int start()
{
    write('h');
    write('e');
    write('l');
    write('l');
    write('o');
}
