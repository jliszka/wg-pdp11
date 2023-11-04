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
    mov r5, sp
    pop r5
    rts pc
.endm


.globl _exit
_exit:
    push r5
    mov sp, r5
    trap 0
    halt    # unreachable

syscall _halt, 1
syscall _fork, 2
syscall _exec, 3
syscall _open, 4
syscall _close, 5
syscall _lseek, 6
syscall _read, 7
syscall _write, 8
syscall _fsync, 9
syscall _link, 10
syscall _unlink, 11
syscall _mkdir, 12
syscall _rmdir, 13
syscall _stat, 14
syscall _mkfs, 15
syscall _wait, 16
syscall _chdir, 17
syscall _getcwd, 18
syscall _pipe, 19
