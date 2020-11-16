#include "libasio.h"

#define KBV 060            // keyboard (TTI) interrupt vector
#define KBS 0177560        // keyboard status register
#define KBD 0177562        // keyboard data buffer
#define KB_READY (1 << 7)  // keyboard ready bit
#define KB_ENABLE (1 << 6) // keyboard interrupt enable bit

#define TTV 064            // terminal (TTO) interrupt vector
#define TTS 0177564        // terminal status register
#define TTD 0177566        // terminal data buffer
#define TT_READY (1 << 7)  // terminal ready bit
#define TT_ENABLE (1 << 6) // terminal interrupt enable bit

#define PTR_STATUS 0177550 // paper tape reader status word
#define PTR_DATA 0177552   // paper tape reader data buffer
#define PTR_ENABLE 1
#define PTR_READY (1 << 7)
#define PTR_ERROR (1 << 15)

#define DEL 0177
#define ESC 033

// This lives in isr.s
extern void isrinit();

unsigned char outbuf[64];
unsigned char *outend = outbuf;
unsigned char *outptr = outbuf;

unsigned char inbuf[64];
unsigned char *inend = inbuf;
unsigned char *inptr = inbuf;

#define kb_enable() \
    *((volatile unsigned int *)KBS) |= KB_ENABLE;

#define kb_disable() \
    *((volatile unsigned int *)KBS) &= ~KB_ENABLE;

#define tt_enable() \
    *((volatile unsigned int *)TTS) |= TT_ENABLE;

#define tt_disable() \
    *((volatile unsigned int *)TTS) &= ~TT_ENABLE;

void io_init()
{
    outend = outbuf;
    outptr = outbuf;
    inend = inbuf;
    inptr = inbuf;

    tt_disable();
    kb_disable();

    isrinit();
    kb_enable();
}

void write(char *str)
{
    while (*str)
    {
        *outend++ = *str++;
    }
}

void writeln(char *str)
{
    write(str);
    write("\r\n");
    flush();
}

void read(char *dst)
{
    while (*(inend - 1) != '\r')
    {
        asm("wait");
    }
    while (*inptr != '\r')
    {
        *dst++ = *inptr++;
    }
    *dst = 0;
    inend = inbuf;
    inptr = inbuf;
}

unsigned char getch() 
{
    while (inptr == inend) 
    {
        asm("wait");
    }
    return *inptr++;
}

void flush()
{
    tt_enable();
    while (outend != outbuf)
    {
        asm("wait");
    }
}

// Keyboard interrupt handler, called from isr.s
void kb_handler()
{
    unsigned char *rbuf = (unsigned char *)KBD;
    register char c = *rbuf;

    if (c == DEL) {
        *outend++ = '\b';
        *outend++ = ' ';
        *outend++ = '\b';
        if (inend > inbuf) inend--;
    } else {
        *outend++ = c;
        *inend++ = c;
    }
    tt_enable();
}

// Terminal interrupt handler, called from isr.s
void tt_handler()
{
    unsigned char *xbuf = (unsigned char *)TTD;

    if (outptr == outend)
    {
        if (outptr != outbuf)
        {
            outptr = outbuf;
            outend = outbuf;
            tt_disable();
        }
    }
    else
    {
        *xbuf = *outptr++;
    }
}

// Paper tape reader interface
int ptr_status = 0;
unsigned char ptr_char;
int ptr_has_next()
{
    if (ptr_status == 1)
        return 1;
    if (ptr_status == -1)
        return 0;
    volatile unsigned int *xcsr = (unsigned int *)PTR_STATUS;
    volatile unsigned char *xbuf = (unsigned char *)PTR_DATA;
    *xcsr |= PTR_ENABLE;
    while ((*xcsr & PTR_READY) == 0)
    {
        if (*xcsr & PTR_ERROR)
        {
            ptr_status = -1;
            return 0;
        }
    }
    ptr_char = *xbuf;
    ptr_status = 1;
    return 1;
}

unsigned char ptr_next()
{
    ptr_status = 0;
    return ptr_char;
}

int ptr_read(int bytes, unsigned char *dst)
{
    volatile unsigned int *xcsr = (unsigned int *)PTR_STATUS;
    volatile unsigned char *xbuf = (unsigned char *)PTR_DATA;
    int byte_count;
    for (byte_count = 0; byte_count < bytes; byte_count++)
    {
        *xcsr |= PTR_ENABLE;
        while ((*xcsr & PTR_READY) == 0)
        {
            if (*xcsr & PTR_ERROR)
            {
                return byte_count;
            }
        }
        *dst++ = *xbuf;
    }
    return byte_count;
}
