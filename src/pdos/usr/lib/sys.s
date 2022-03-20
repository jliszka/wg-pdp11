.title "System calls"

.text
.even

.include "macros.s"

.macro syscall fn tr
.globl \fn
\fn:
	push r5
	mov sp, r5
	trap \tr
	pop r5
	rts pc
.endm


.globl _exit
_exit:
	push r5
	mov sp, r5
	trap 0
	pop r5
1$:
	br 1$  	# unreachable

syscall _halt, 1
syscall _fork, 2
syscall _exec, 3
syscall _fopen, 4
syscall _fclose, 5
syscall _fseek, 6
syscall _fread, 7
syscall _fwrite, 8
syscall _fflush, 9
syscall _link, 10
syscall _unlink, 11
syscall _mkdir, 12
syscall _rmdir, 13
syscall _stat, 14
syscall _mkfs, 15
