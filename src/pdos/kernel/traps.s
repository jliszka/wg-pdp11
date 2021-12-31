.title "Kernel traps"

.include "macros.s"

.globl _tty_read
.globl _tty_write
.globl _tty_flush
.globl _userexec

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
	.word trap.halt		# 1
	.word trap.fork 	# 2
	.word trap.exec 	# 3
	.word trap.fopen	# 4
	.word trap.fclose	# 5
	.word trap.fseek	# 6
	.word trap.fread        # 7
	.word trap.fwrite       # 8
	.word trap.fflush       # 9
	.word trap.link 	# 10
	.word trap.unlink	# 11
	.word trap.mkdir        # 12
	.word trap.rmdir	# 13


# "Reverse" trap to jump to user mode program
_userexec:
    push r1
    push r2
    push r3
    push r4
    push r5

    push $0140000
    push $020000
    rti                 # simultaneously set mode and "jump"

# r5 points to user-space stack:
#     - exit code
#     - return address for call to syscall stub
# r5 -> old r5
trap.exit:
    mfpi 4(r5)
    pop r0
	# current stack looks like:
    #     - return address for call to userexec()
    #     - saved registers r1-r5
    #     - PSW for this trap
    #     - return address for this trap
    #     - r2
    # sp -> r3
	# We want to ignore the first 4, restore the registers, and then return,
    # effectively simulating the "return" from userexec()
	add $8, sp

    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    rts pc

# No arguments
trap.halt:
    halt
	jmp ret

# No arguments
trap.fork:
    mov $-1, r0
	jmp ret

# r5 points to user-space stack:
#     - arguments (char **)
#     - path to executable
#     - return address for call to syscall stub
# r5 -> old r5
trap.exec:
    mov $-1, r0
	jmp ret

# r5 points to user-space stack:
#     - mode (char)
#     - path (char *)
#     - return address for call to syscall stub
# r5 -> old r5
trap.fopen:
    mfpi 4(r5)          # path string
    push $buf
    jsr pc, readbuf
    add $4, sp

    mfpi 6(r5)          # mode
    push $buf
    jsr pc, _io_fopen
    add $4, sp

    jmp ret


# Read from user-space buffer (top of stack)
readbuf:
	mov 2(sp), r0       # buffer to copy into
	mov 4(sp), r1	    # user-space pointer

	bit r1, $1          # check if the address is even
	beq 1$

	dec r1              # if odd, copy only the first byte
	mfpi (r1)+
	pop r2
	br 2$

1$:
	mfpi (r1)+          # read a word from the previous address space
	pop r2
	movb r2, (r0)+      # copy the low byte to destination buffer
	beq 3$              # if we copied a 0 byte, we're done
2$:
	ash $-8, r2
	movb r2, (r0)+      # copy the high byte to destination buffer
	beq 3$              # if we copied a 0 byte, we're done

	br 1$               # go around again!

3$:
	rts pc


# r5 points to user-space stack:
#     - fd
#     - return address for call to syscall stub
# r5 -> old r5
trap.fclose:
    mfpi 4(r5)
    jsr pc, _io_fclose
    add $2, sp
    jmp ret

# r5 points to user-space stack:
#     - pos
#     - fd
#     - return address for call to syscall stub
# r5 -> old r5
trap.fseek:
    mfpi 6(r5)
    mfpi 4(r5)
    jsr pc, _io_fseek
    add $4, sp
    jmp ret

# r5 points to user-space stack:
#     - len
#     - dest buffer
#     - fd
#     - return address for call to syscall stub
# r5 -> old r5
trap.fread:
    push r5
    mfpi 8(r5)          # number of bytes to read
    push $buf           # destination buffer
    mfpi 4(r5)          # file descriptor
    jsr pc, _io_fread   # return value (r0): how many bytes read
    add $6, sp
    pop r5

    push r0             # save original value of r0
    beq 4$              # no bytes read, return

    mfpi 6(r5)          # destination buffer => r1
    pop r1

    mov $buf, r2
    bit r1, $1          # check if the address is even
    beq 1$

    dec r1
    mfpi (r1)           # copy bytes from userspace destination buffer to stack
    mov sp, r3          # use r3 instead of sp because sp can only point to even addresses
    inc r3
    br 2$

1$:
    mfpi (r1)           # copy bytes from userspace destination buffer to stack
    mov sp, r3
    movb (r2)+, (r3)+   # overwrite with data from $buf
    dec r0
    beq 3$

2$:
    movb (r2)+, (r3)+
    dec r0
    beq 3$
    mtpi (r1)+          # copy it back to userspace destination buffer
    br 1$
3$:
    mtpi (r1)+          # copy final word to userspace destination buffer

4$:
    pop r0              # number of bytes read
    jmp ret

# r5 points to user-space stack:
#     - len
#     - src buffer
#     - fd
#     - return address for call to syscall stub
# r5 -> old r5
trap.fwrite:
    mfpi 8(r5)          # number of bytes to write
    pop r0
    mfpi 6(r5)          # source buffer
    pop r1

    tst r0              # nothing to write? return
    beq ret

    cmp r0, $bufsize    # don't copy over more than the size of the buffer
    ble 4$
    mov $bufsize, r0

4$:
    push r0             # save the number of bytes to write
    mov $buf, r3
    bit r1, $1          # check if the address is even
    beq 1$

    dec r1              # if odd, copy only the first byte
    mfpi (r1)+
    pop r2
    br 2$

1$:
    mfpi (r1)+          # read a word from the previous address space
    pop r2
    movb r2, (r3)+      # copy the low byte to destination buffer
    dec r0
    beq 3$
2$:
    ash $-8, r2
    movb r2, (r3)+      # copy the high byte to destination buffer
    dec r0
    beq 3$
    br 1$

3$:
                        # number of bytes to write is already on the top of the stack
    push $buf           # destination buffer
    mfpi 4(r5)          # file descriptor
    jsr pc, _io_fwrite
    add $6, sp

    jmp ret

# r5 points to user-space stack:
#     - fd
#     - return address for call to syscall stub
# r5 -> old r5
trap.fflush:
    mfpi 4(r5)
    jsr pc, _io_fflush
    add $2, sp
    jmp ret


# r5 points to user-space stack:
#     - target path name (user-space pointer)
#     - source path name (user-space pointer)
#     - return address for call to syscall stub
# r5 -> old r5
trap.link:
	mfpi 4(r5)          # source path
	push $buf
	jsr pc, readbuf
	add $4, sp

	push r0	            # r0 is the beginning of the target path string

	mfpi 6(r5)          # target path
	push r0
	jsr pc, readbuf
	add $4, sp

	# target path is already on the stack
	push $buf          # source path
	jsr pc, _fs_link
	add $4, sp

	jmp ret


# r5 points to user-space stack:
#     - target path name (user-space pointer)
#     - return address for call to syscall stub
# r5 -> old r5
trap.unlink:
	mfpi 4(r5)          # target dir name
	push $buf
	jsr pc, readbuf
	add $4, sp

	push $buf
	jsr pc, _fs_unlink
	add $2, sp

	jmp ret

# r5 points to user-space stack:
#     - target dir name (user-space pointer)
#     - return address for call to syscall stub
# r5 -> old r5
trap.mkdir:
	mfpi 4(r5)          # target dir name
	push $buf
	jsr pc, readbuf
	add $4, sp

	push $buf
	jsr pc, _fs_mkdir
	add $2, sp

	jmp ret

# r5 points to user-space stack:
#     - target dir name (user-space pointer)
#     - return address for call to syscall stub
# r5 -> old r5
trap.rmdir:
	mfpi 4(r5)          # target dir name
	push $buf
	jsr pc, readbuf
	add $4, sp

	push $buf
	jsr pc, _fs_rmdir
	add $2, sp

	jmp ret

