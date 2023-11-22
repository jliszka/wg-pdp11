
.text
.even

.include "macros.s"

kbv = 060
ttv = 064
trapv = 034
rkbv = 0300
rttv = 0304

.globl _kb_handler          # these are C functions that live in libasio.c
.globl _tt_handler
.globl ktrap


.macro isr n
kbisr\n:
    push r0
    mov $\n, r0
    jmp kbisr

ttisr\n:
    push r0
    mov $\n, r0
    jmp ttisr

.endm

    isr 0
    isr 1
    isr 2
    isr 3


.globl _isrinit
_isrinit:                   # this is called from libasio.c as isrinit()
    mov $kbisr0, @$kbv      # install the keyboard interrupt handler
    mov $ttisr0, @$ttv      # install the terminal interrupt handler
    mov $kbisr1, @$rkbv
    mov $ttisr1, @$rttv
    mov $ktrap, @$trapv     # install the kernel trap handler
    rts pc


# All these interrupt handlers do is save the registers, call back into
# C-land, and rti (return from interrupt). These need to be in assembly
# because GCC won't generate these instructions for a normal function.
kbisr:
    push r1
    push r2
    push r3
    push r4
    push r5

    push r0                 # device number
    jsr pc, _kb_handler     # call the C-land interrupt handler
    add $2, sp

    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0

    rti

ttisr:
    push r1
    push r2
    push r3
    push r4
    push r5

    push r0                 # device number
    jsr pc, _tt_handler     # call the C-land interrupt handler
    add $2, sp

    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0

    rti

