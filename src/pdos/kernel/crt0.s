#############################################################################*
#####
##### crt0.s contains the initial boot routines for C programs
#####
######

.text
.even
.globl _main
.globl ___main
.globl _start

#############################################################################*
##### _start: initialize stack pointer,
#####         clear vector memory area,
#####         save program entry in vector 0
#####         call C main() function
#############################################################################*
_start:
    mov $0160000,sp
    clr r0
L_0:
    # Uncomment these to add fake interrupt handlers for debugging
    # mov r0, (r0)
    # add $060000, (r0)+   # will trap to 060000 + vector address
    clr (r0)+
    cmp r0, $0400
    bne L_0
    jsr pc,_main
    halt
    br _start

#############################################################################*
##### ___main: called by C main() function. Currently does nothing
#############################################################################*
___main:
    rts pc
