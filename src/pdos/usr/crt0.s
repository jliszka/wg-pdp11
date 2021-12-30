#############################################################################*
#####
##### crtu.s contains the initial boot routines for userland C programs
#####
######

.text
.even
.globl _main
.globl ___main
.globl _start

#############################################################################*
##### _start: initialize stack pointer,
#####         call C main() function
#############################################################################*
_start:
    mov $0177674, sp
    jsr pc,_main
    mov r0,-(sp)  # push return code
    jsr pc,_exit  # exit

#############################################################################*
##### ___main: called by C main() function. Currently does nothing
#############################################################################*
___main:
    rts pc
