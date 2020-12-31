.title "System calls"

.text
.even

.include "macros.s"

.globl _exit
_exit:
	push r5
	mov sp, r5
	trap 0
	pop r5
1$:
	br 1$  	# unreachable

.globl _read
_read:
	push r5
	mov sp, r5
	trap 1
	pop r5
	rts pc

.globl _write
_write:
	push r5
	mov sp, r5
	trap 2
	pop r5
	rts pc

.globl _flush
_flush:
	trap 3
	rts pc
