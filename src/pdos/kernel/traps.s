.title "Kernel traps"

.include "macros.s"

.globl _read
.globl _write
.globl _flush

.text
.even

.globl ktrap
ktrap:
	push r2
	push r3
	mov 4(sp), r2
	sub $2, r2
	clr r3
	movb (r2), r3
	asl r3
	add $ttable, r3
	jmp @(r3)

ret:
	pop r3
	pop r2
	rti

buf:
.=.+64

ttable:
	.word trap.exit
	.word trap.read
	.word trap.write
	.word trap.flush

trap.exit:
	jmp ret

trap.read:
	jmp ret

trap.write:
	mov $buf, r2
1$:
	mfpi (r1)+
	pop (r2)+
	tstb -2(r2)
	beq 2$
	tstb -1(r2)
	beq 2$
	br 1$

2$:
	push $buf
	jsr pc, _write
	add $2, sp
	
	jmp ret

trap.flush:
	jsr pc, _flush
	jmp ret


