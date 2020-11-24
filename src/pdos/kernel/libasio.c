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

#define BUFSIZE 64
unsigned char outbuf[BUFSIZE];
volatile unsigned int outend = 0;
volatile unsigned int outptr = 0;

unsigned char inbuf[BUFSIZE];
volatile unsigned int inend = 0;
volatile unsigned int inptr = 0;

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
    outend = 0;
    outptr = 0;
    inend = 0;
    inptr = 0;

    tt_disable();
    kb_disable();

    isrinit();
    kb_enable();
}

void write(char *str)
{
    while (*str)
    {
        outbuf[outend] = *str++;
        outend = (outend + 1) % BUFSIZE;
    }
}

void writeln(char *str)
{
    write(str);
    write("\r\n");
    flush();
}

int read(int nbytes, char *dst)
{
    int len = 0;
    while (inbuf[inend-1] != '\r')
    {
        asm("wait");
    }
    while (inbuf[inptr] != '\r' && len < nbytes-1 && len < BUFSIZE)
    {
        *dst++ = inbuf[inptr++];
        len++;
    }
    *dst = 0;
    inend = 0;
    inptr = 0;
    return len+1;
}

unsigned char getch() 
{
    while (inptr == inend) 
    {
        asm("wait");
    }
    return inbuf[inptr++];
}

void flush()
{
    tt_enable();
    while (outptr != outend)
    {
        asm("wait");
    }
}

// Keyboard interrupt handler, called from isr.s
void kb_handler()
{
    volatile unsigned char *rbuf = (unsigned char *)KBD;
    register char c = *rbuf;

    if (c == DEL) {
        if (inend > 0) {
            outbuf[(outend++)%BUFSIZE] = '\b';
            outbuf[(outend++)%BUFSIZE] = ' ';
            outbuf[(outend++)%BUFSIZE] = '\b';
            inend--;
        }
    } else if (c == ESC) {
        ; // Don't echo or store escape codes (e.g., arrow keys)
    } else if (c == '\r') {
        outbuf[(outend++)%BUFSIZE] = '\r';
        outbuf[(outend++)%BUFSIZE] = '\n';        
        inbuf[inend++] = c;
    } else {
        outbuf[(outend++)%BUFSIZE] = c;
        inbuf[inend++] = c;
    }
    outend = outend % BUFSIZE;
    tt_enable();
}

// Terminal interrupt handler, called from isr.s
void tt_handler()
{
    volatile unsigned char *xbuf = (unsigned char *)TTD;

    if (outptr == outend)
    {
        tt_disable();
    }
    else
    {
        *xbuf = outbuf[outptr];
        outptr = (outptr + 1) % BUFSIZE;
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
