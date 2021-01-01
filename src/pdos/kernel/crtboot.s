#############################################################################*
#####
##### crtboot.s contains the initial boot routines for the bootstrap loader C program
#####
######

.text
.even
.globl _main
.globl ___main
.globl _start

# The inode number of the kernel program. This gets overwritten
# by the mbr command when it installs the bootloader.
.globl _kernel_inode
_kernel_inode:
.word 0

#############################################################################*
##### _start: initialize stack pointer,
#####         clear vector memory area,
#####         call C main() function
#############################################################################*
_start:
    mov $0160000,sp
    clr r0
L_0:
    clr (r0)+
    cmp r0, $0400
    bne L_0
    jsr pc,_main
    halt

#############################################################################*
##### ___main: called by C main() function. Currently does nothing
#############################################################################*
___main:
    rts pc
