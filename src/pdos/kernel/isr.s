
.text
.even

.include "macros.s"

kbv = 060
ttv = 064
trapv = 034

.globl _kb_handler          # these are C functions that live in libasio.c
.globl _tt_handler
.globl ktrap

.globl _isrinit
_isrinit:                   # this is called from libasio.c as isrinit()
    mov $kbisr, @$kbv       # install the keyboard interrupt handler
    mov $ttisr, @$ttv       # install the terminal interrupt handler
    mov $ktrap, @$trapv     # install the kernel trap handler
    rts pc

# All these interrupt handlers do is save the registers, call back into
# C-land, and rti (return from interrupt). These need to be in assembly
# because GCC won't generate these instructions for a normal function.
kbisr:
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5

    jsr pc, _kb_handler     # call the C-land interrupt handler

    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0

    rti

ttisr:
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5

    jsr pc, _tt_handler     # call the C-land interrupt handler

    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0

    rti

