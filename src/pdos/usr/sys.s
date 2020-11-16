.title "System calls"

.text
.even

.include "macros.s"

.globl _exit
_exit:
	mov 2(sp), r0
	trap 0
1$:
	br 1$  	# unreachable

.globl _read
_read:
	mov 2(sp), r0
	mov 4(sp), r1

	trap 1

	rts pc

.globl _write
_write:
	mov 2(sp), r0
	mov 4(sp), r1

	trap 2

	rts pc

.globl _flush
_flush:
	trap 3
	rts pc
