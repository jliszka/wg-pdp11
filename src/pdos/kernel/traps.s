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
	mov 4(sp), r2		# return address
	sub $2, r2			# subtract 2 to get the trap instruction
	mfpi (r2)			# read the trap instruction (from previous address space)
	pop r2
	movb r2, r3			# get the low byte of the instruction to determine the trap number
	asl r3				# multiply by 2...
	jmp @ttable(r3)		# and address into the jump table

ret:
	pop r3
	pop r2
	rti

.even
bufsize = 64
buf:
.=.+bufsize

ttable:
	.word trap.exit
	.word trap.read
	.word trap.write
	.word trap.flush

trap.exit:
	# current stack looks like:
	# - r3
	# - r2
	# - return address for this trap
	# - PSW for this trap
	# - return address for jsr to call user main(
	# We want to ignore the first 4 and then return, so it'll be as if
	# the jsr to user main() "returned" into kernel mode.
	add $8, sp
	rts pc

trap.read:
	push r1

	push $buf
	push r0
	jsr pc, _read		# return value (r0): how many bytes read
	add $4, sp

	pop r1
	push r0				# save original value of r0

	mov $buf, r2
	bit r1, $1			# check if the address is even
	beq 1$

	dec r1
	mfpi (r1)			# copy bytes from userspace destination buffer to stack
	mov sp, r3			# use r3 instead of sp because sp can only point to even addresses
	inc r3
	br 2$

1$:
	mfpi (r1)			# copy bytes from userspace destination buffer to stack
	mov sp, r3
	movb (r2)+, (r3)+	# overwrite with data from $buf
	dec r0
	beq 3$

2$:
	movb (r2)+, (r3)+
	dec r0
	beq 3$
	mtpi (r1)+			# copy it back to userspace destination buffer
	br 1$
3$:
	mtpi (r1)+			# copy final word to userspace destination buffer

	pop r0				# number of bytes read
	jmp ret

trap.write:
	tst r0				# nothing to write? return
	beq ret

	cmp r0, $bufsize	# don't copy over more than the size of the buffer
	ble 4$
	mov $bufsize, r0

4$:
	push r0
	mov $buf, r3
	bit r1, $1			# check if the address is even
	beq 1$

	dec r1				# if so, copy only the first byte
	mfpi (r1)+
	pop r2
	br 2$

1$:
	mfpi (r1)+			# read a word from the previous address space
	pop r2
	movb r2, (r3)+		# copy the low byte to destination buffer
	dec r0
	beq 3$
2$:
	ash $-8, r2
	movb r2, (r3)+		# copy the high byte to destination buffer
	dec r0
	beq 3$
	br 1$

3$:
	push $buf
	jsr pc, _write
	add $2, sp

	pop r0
	jmp ret

trap.flush:
	jsr pc, _flush
	jmp ret


