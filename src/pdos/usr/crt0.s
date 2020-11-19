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
    # clr sp
L_0:
    jsr pc,_main
    clr r0
    rts pc
    # trap 0 		# exit

#############################################################################*
##### ___main: called by C main() function. Currently does nothing
#############################################################################*
___main:
    rts pc
