.title "Kernel traps"

.include "macros.s"

.globl _tty_read
.globl _tty_write
.globl _tty_flush

.text
.even

# The syscall stubs in sys.s set it up so that r5 points to the user stack.
# Traps handlers can access the first argument as 4(r5), the second as 6(r5), etc.
#     - arg3
#     - arg2
#     - arg1
#     - return address for call to syscall stub
# r5 -> old r5
.globl ktrap
ktrap:
	mov (sp), r0		# return address for the trap call (in user address space)
	sub $2, r0			# subtract 2 to get the address of the trap instruction
	mfpi (r0)			# read the trap instruction (from previous instruction space = user)
	pop r0
	bic $0177400, r0	# get the low byte of the instruction to determine the trap number
	asl r0				# multiply by 2...

    push r2
    push r3
	jmp @ttable(r0)		# and address into the jump table

ret:
    pop r3
    pop r2
	rti

.even
bufsize = 64
buf:
.=.+bufsize

ttable:
	.word trap.exit		# 0
	.word trap.read		# 1
	.word trap.write	# 2
	.word trap.flush	# 3
	#.word trap.fopen	# 4
	#.word trap.fclose	# 5
	#.word trap.fseek	# 6
    #.word trap.fread   # 7
    #.word trap.fwrite  # 8
	#.word trap.link 	# 9
	#.word trap.unlink	# 10

# r5 points to user-space stack:
#     - exit code
#     - return address for call to syscall stub
# r5 -> old r5
trap.exit:
    mfpi 4(r5)
    pop r0
	# current stack looks like:
    #     - return address for jsr to call user main()
    #     - PSW for this trap
    #     - return address for this trap
    #     - r2
    # sp -> r3
	# We want to ignore the first 2 and then return, so it'll be as if
	# the jsr to user main() "returned" into kernel mode.
	add $8, sp
	rts pc

# r5 points to user-space stack:
#     - destination buffer
#     - number of bytes to read
#     - return address for call to syscall stub
# r5 -> old r5
trap.read:
	mfpi 4(r5)			# number of bytes to read
	pop r0

	push $buf
	push r0
	jsr pc, _tty_read	# return value (r0): how many bytes read
	add $4, sp

	push r0				# save original value of r0

    mfpi 6(r5)          # destination buffer
    pop r1

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

# r5 points to user-space stack:
#     - source buffer
#     - number of bytes to write
#     - return address for call to syscall stub
# r5 -> old r5
trap.write:
    mfpi 4(r5)          # number of bytes to write
    pop r0
    mfpi 6(r5)          # source buffer
    pop r1

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
	pop r0
	push $buf
	push r0
	jsr pc, _tty_write
	add $4, sp

	jmp ret

# No arguments
trap.flush:
	jsr pc, _tty_flush
	jmp ret


