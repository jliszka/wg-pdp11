.title "System calls"

.text
.even

.include "macros.s"

.globl _exit
_exit:
	mov sp, r5
	add $2, r5
	trap 0
1$:
	br 1$  	# unreachable

.globl _read
_read:
	mov sp, r5
	add $2, r5
	trap 1
	rts pc

.globl _write
_write:
	mov sp, r5
	add $2, r5
	trap 2
	rts pc

.globl _flush
_flush:
	trap 3
	rts pc
