#include "libasio.h"

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

// This lives in isr.s
extern void isrinit();

unsigned char outbuf[64];
unsigned char * outend = outbuf;
unsigned char * outptr = outbuf;

unsigned char inbuf[64];
unsigned char * inptr = inbuf;

void io_init() {
    outend = outbuf;
    outptr = outbuf;
    inptr = inbuf;

    volatile unsigned int *xcsr = (unsigned int *)KBS;
    *xcsr |= KB_ENABLE;

    isrinit();
}

void write(char * str) {
    while (*str) {
        *outend++ = *str++;
    }
}

void read(char * dst) {
    while (*(inptr-1) != '\r') {
        asm("wait");
    }
    char * p = inbuf;
    while (*p != '\r') {
        *dst++ = *p++;
    }
    *dst = 0;
    inptr = inbuf;
}

void flush() {
    volatile unsigned int *xcsr = (unsigned int *)TTS;
    *xcsr |= TT_ENABLE;
    while (outend != outbuf) {
        asm("wait");
    }
}

// Keyboard interrupt handler, called from isr.s
void kb_handler() {
    unsigned char *rbuf = (unsigned char *)KBD;
    unsigned char *xbuf = (unsigned char *)TTD;
    register char c = *rbuf;
    *xbuf = c;
    *inptr++ = c;
}

// Terminal interrupt handler, called from isr.s
void tt_handler() {
    volatile unsigned int *xcsr = (unsigned int *)TTS;
    unsigned char *xbuf = (unsigned char *)TTD;

    if (outptr == outend) {
        if (outptr != outbuf) {
            outptr = outbuf;
            outend = outbuf;
            *xcsr &= ~TT_ENABLE;
        }
    } else {
        *xbuf = *outptr++;
    }
}
