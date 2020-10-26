#############################################################################*
#####
##### crt0.s contains the initial boot routines for C programs
#####
######

.text
.even
.globl	_main
.globl	___main
.globl	_start

#############################################################################*
##### _start: initialize stack pointer,
#####         clear vector memory area,
#####         save program entry in vector 0
#####         call C main() function
#############################################################################*
_start:
	mov	$00776,sp
	mov	r2,r3
	mov	r3,r4
	mov	r4,r5
	clr	r0
L_0:
	clr	(r0)+
	cmp	r0, $400
	bne	L_0
        mov	$000137,*$0     # Store JMP _start in vector 0
        mov	$_start,*$2
	jsr 	pc,_main
	halt
        br	_start

#############################################################################*
##### ___main: called by C main() function. Currently does nothing
#############################################################################*
___main:
	rts	pc
