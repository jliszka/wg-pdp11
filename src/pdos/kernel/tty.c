#include "tty.h"
#include "proc.h"

#define KBV 060            // keyboard (TTI) interrupt vector
#define KBS 0177560        // keyboard status register
#define KBD 0177562        // keyboard data buffer
#define KB_READY (1 << 7)  // keyboard ready flag
#define KB_RING (1 << 14)  // terminal ring flag
#define KB_ENABLE (1 << 6) // keyboard interrupt enable bit
#define KB_RING_INT (1 << 5) // keyboard ring interrupt enable bit

#define DL 0176500         // remote terminal status and data

#define TTV 064            // terminal (TTO) interrupt vector
#define TTS 0177564        // terminal status register
#define TTD 0177566        // terminal data buffer
#define TT_READY (1 << 7)  // terminal ready bit
#define TT_ENABLE (1 << 6) // terminal interrupt enable bit

#define DEL 0177
#define ESC 033
#define CONT 024

// This lives in isr.s
extern void isrinit();

#define BUFSIZE 64
#define MAX_ALLOWANCE 32

typedef struct {
    unsigned char outbuf[BUFSIZE];
    unsigned int outend;
    unsigned int outptr;

    unsigned char inbuf[BUFSIZE];
    unsigned int inend;
    unsigned int inptr;

    int send_allowance;
} tty_t;

#define MAX_TTY 4
tty_t ttys[MAX_TTY];

typedef struct {
    int rx_status;
    int rx_buffer;
    int tx_status;
    int tx_buffer;
} tty_device_t;

#define tty_devices ((tty_device_t *)DL)

void kb_enable(int tty) {
    if (tty == 0) *((volatile unsigned int *)KBS) |= KB_ENABLE;
    else tty_devices[tty-1].rx_status |= KB_ENABLE | KB_RING_INT;
}

void kb_disable(int tty) {
    if (tty == 0) *((volatile unsigned int *)KBS) &= ~KB_ENABLE;
    else tty_devices[tty-1].rx_status &= ~(KB_ENABLE | KB_RING_INT);
}

void tt_enable(int tty) {
    if (tty == 0) *((volatile unsigned int *)TTS) |= TT_ENABLE;
    else tty_devices[tty-1].tx_status |= TT_ENABLE;
}

void tt_disable(int tty) {
    if (tty == 0) *((volatile unsigned int *)TTS) &= ~TT_ENABLE;
    else tty_devices[tty-1].tx_status &= ~TT_ENABLE;
}

void tty_init() {
    for (int i = 0; i < MAX_TTY; i++) {
        ttys[i].outend = 0;
        ttys[i].outptr = 0;
        ttys[i].inend = 0;
        ttys[i].inptr = 0;
        ttys[i].send_allowance = MAX_ALLOWANCE;
    }

    tt_disable(0);
    kb_disable(0);

    isrinit();
    kb_enable(0);
    kb_enable(1);
}

int tty_write(int tty, int nbytes, char *str) {
    tty_t *ptty = &ttys[tty];
    asm("spl 7");
    if (ptty->outend + nbytes > ptty->outptr + BUFSIZE) {
        nbytes = ptty->outptr + BUFSIZE - ptty->outend;
    }
    asm("spl 0");
    if (nbytes == 0) {
        tty_flush(tty);
        return 0;
    }

    int len = nbytes;
    while (nbytes-- > 0)
    {
        ptty->outbuf[ptty->outend++ % BUFSIZE] = *str;
        if (*str == '\n') {
            tty_flush(tty);
        }
        str++;
    }
    return len;
}

int tty_read(int tty, int nbytes, char *dst, char *endch) {
    tty_t *ptty = &ttys[tty];
    int len = 0;
    while (1) {
        char last_char = ptty->inbuf[ptty->inend-1];
        if (last_char == '\n' || last_char == CTRL_D) break;
        proc_block();
    }

    char c;
    while (len < nbytes && len < BUFSIZE) {
        c = ptty->inbuf[ptty->inptr];
        if (c == CTRL_D) break;
        if (c == '\n') break;
        *dst++ = c;
        ptty->inptr++;
        len++;
    }
    *endch = c;
    ptty->inend = 0;
    ptty->inptr = 0;
    return len;
}

unsigned char tty_getch(int tty) {
    tty_t *ptty = &ttys[tty];
    while (ptty->inptr == ptty->inend) {
        proc_block();
    }
    return ptty->inbuf[ptty->inptr++];
}

void tty_flush(int tty) {
    tty_t *ptty = &ttys[tty];

    while (tty > 0 && !(tty_devices[tty-1].rx_status & KB_RING)) {
        // No one's listening; block
        proc_block();
    }

    if (ptty->outptr != ptty->outend) {
        tt_disable(tty);
        tt_enable(tty);
        while (ptty->outptr != ptty->outend) {
            asm("wait");
        }
    }
}

// Keyboard interrupt handler, called from isr.s
void kb_handler(int tty) {
    unsigned char c;
    if (tty == 0) {
        c = *(char *)KBD;
    } else {
        c = (unsigned char)tty_devices[tty-1].rx_buffer;
        if ((tty_devices[tty-1].rx_status & KB_RING) && (c == 0)) {
            // Ring!
            tt_enable(tty);
            return;
        }
    }

    tty_t *ptty = &ttys[tty];

    if (c == DEL || c == '\b') {
        if (ptty->inend > 0) {
            ptty->outbuf[ptty->outend++ % BUFSIZE] = '\b';
            ptty->outbuf[ptty->outend++ % BUFSIZE] = ' ';
            ptty->outbuf[ptty->outend++ % BUFSIZE] = '\b';
            ptty->inend--;
        }
    } else if (c == ESC) {
        ; // Don't echo or store escape codes (e.g., arrow keys)
    } else if (c == '\r') {
        ptty->outbuf[ptty->outend++ % BUFSIZE] = '\r';
        ptty->outbuf[ptty->outend++ % BUFSIZE] = '\n';
        ptty->inbuf[ptty->inend++] = '\r';
        ptty->inbuf[ptty->inend++] = '\n';
    } else if (c == CONT && tty == 0) {
        ptty->send_allowance = MAX_ALLOWANCE;
        tty_flush(tty);
        return;
    } else {
        ptty->outbuf[ptty->outend++ % BUFSIZE] = c;
        ptty->inbuf[ptty->inend++] = c;
    }
    tt_enable(tty);
}

// Terminal interrupt handler, called from isr.s
void tt_handler(int tty) {
    unsigned char *xbuf;
    if (tty == 0) {
        xbuf = (unsigned char *)TTD;
    } else {
        xbuf = (unsigned char *)(&(tty_devices[tty-1].tx_buffer));
    }
    tty_t *ptty = &ttys[tty];

    if (ptty->outptr == ptty->outend || ptty->send_allowance <= 0) {
        tt_disable(tty);
    } else {
        *xbuf = ptty->outbuf[ptty->outptr++ % BUFSIZE];
        if (tty == 0) {
            ptty->send_allowance--;
        }
    }
}

