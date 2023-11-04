.title "Kernel traps"

.include "macros.s"

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
    mov (sp), r0        # return address for the trap call (in user address space)
    sub $2, r0          # subtract 2 to get the address of the trap instruction
    mfpi (r0)           # read the trap instruction (from previous instruction space = user)
    pop r0
    bic $0177400, r0    # get the low byte of the instruction to determine the trap number
    asl r0              # multiply by 2...

    push r2
    push r3
    push r4
    push r5             # (user stack pointer)
    jmp @ttable(r0)     # and address into the jump table

ret:
    pop r5              # (user stack pointer)
    pop r4
    pop r3
    pop r2

    rti

.even
bufsize = 64
buf:
.=.+bufsize

ttable:
    .word trap.exit     # 0
    .word trap.halt     # 1
    .word trap.fork     # 2
    .word trap.exec     # 3
    .word trap.open     # 4
    .word trap.close    # 5
    .word trap.lseek    # 6
    .word trap.read     # 7
    .word trap.write    # 8
    .word trap.fsync    # 9
    .word trap.link     # 10
    .word trap.unlink   # 11
    .word trap.mkdir    # 12
    .word trap.rmdir    # 13
    .word trap.stat     # 14
    .word trap.mkfs     # 15
    .word trap.wait     # 16
    .word trap.chdir    # 17
    .word trap.getcwd   # 18
    .word trap.pipe     # 19
    .word trap.dup2     # 20


# r5 points to user-space stack:
#     - exit code
#     - return address for call to syscall stub
# r5 -> old r5
trap.exit:
    mfpi 4(r5)
    pop r0
    # current stack looks like:
    #     - return address for call to userexec()
    #     - saved kernel registers r2-r5
    #     - PSW for this trap
    #     - return address for this trap
    # sp -> saved user registers r2-r5
    # We want to ignore the first 6, restore the kernel registers, and then return,
    # effectively "returning" from userexec()
    add $12, sp

    pop r5
    pop r4
    pop r3
    pop r2

    rts pc

# No arguments
trap.halt:
    halt
    jmp ret

# No arguments
trap.fork:
    push $2$                     # return address
    push r2                     # set up the stack for the child
    push r3
    push r4
    push r5

    mov sp, r0
    push r0                     # ksp
    push r5                     # user sp
    jsr pc, _proc_dup
    add $14, sp
1$:
    jmp ret
2$:
    clr r0                      # child return value
    jmp ret

# r5 points to user-space stack:
#     - argv (char**)
#     - argc
#     - return address for call to syscall stub
# r5 -> old r5
trap.exec:
    mfpi 4(r5)          # argc
    pop r4

    mfpi 6(r5)          # argv
    pop r2

    mov r4, r5          # save original argc

    mov $buf, r3        # argv dest
    mov r4, r0          # arg dest = argc * 2 + buf
    asl r0
    add $buf, r0

1$:
    mov r0, (r3)+       # record start of string in argv

    mfpi (r2)+
    push r0
    jsr pc, readbuf     # copy from (r3) in user space to r0, update r0
    add $4, sp

    sob r4, 1$

    push $buf
    push r5             # argc
    jsr pc, _proc_exec

    add $4, sp

    jmp ret

# r5 points to user-space stack:
#     - mode (char)
#     - path (char *)
#     - return address for call to syscall stub
# r5 -> old r5
trap.open:
    mfpi 4(r5)          # path string
    push $buf
    jsr pc, readbuf
    add $4, sp

    mfpi 6(r5)          # mode
    push $buf
    jsr pc, _io_open
    add $4, sp

    jmp ret


# r5 points to user-space stack:
#     - fd
#     - return address for call to syscall stub
# r5 -> old r5
trap.close:
    mfpi 4(r5)
    jsr pc, _io_close
    add $2, sp
    jmp ret

# r5 points to user-space stack:
#     - pos
#     - fd
#     - return address for call to syscall stub
# r5 -> old r5
trap.lseek:
    mfpi 6(r5)
    mfpi 4(r5)
    jsr pc, _io_lseek
    add $4, sp
    jmp ret

# r5 points to user-space stack:
#     - len
#     - dest buffer
#     - fd
#     - return address for call to syscall stub
# r5 -> old r5
trap.read:
    push r5
    mfpi 8(r5)          # number of bytes to read
    pop r0
    tst r0              # nothing to read? return
    beq 3$

    cmp r0, $bufsize    # don't copy over more than the size of the buffer
    ble 1$
    mov $bufsize, r0

1$:
    push r0             # number of bytes to read
    push $buf           # destination buffer
    mfpi 4(r5)          # file descriptor
    jsr pc, _io_read   # return value (r0): how many bytes read
    add $6, sp

    tst r0              # error from io_fread, return
    blt 3$

    pop r5

    push r0             # save original value of r0
    beq 2$              # no bytes read, return

    mfpi 6(r5)          # destination buffer in user space
    push $buf           # source buffer in kernel space
    jsr pc, writebuf
    add $4, sp

2$:
    pop r0              # number of bytes read
3$:
    jmp ret



# r5 points to user-space stack:
#     - len
#     - src buffer
#     - fd
#     - return address for call to syscall stub
# r5 -> old r5
trap.write:
    mfpi 8(r5)          # number of bytes to write
    pop r0
    mfpi 6(r5)          # source buffer
    pop r1

    tst r0              # nothing to write? return
    beq 5$

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
    jsr pc, _io_write
    add $6, sp
5$:
    jmp ret

# r5 points to user-space stack:
#     - fd
#     - return address for call to syscall stub
# r5 -> old r5
trap.fsync:
    mfpi 4(r5)
    jsr pc, _io_fsync
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

    push r0             # r0 is the beginning of the target path string

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


# r5 points to user-space stack:
#     - dest struct pointer
#     - file path
#     - return address for call to syscall stub
# r5 -> old r5
trap.stat:
    push r5

    mfpi 4(r5)          # read file path from user space
    push $buf+6
    jsr pc, readbuf
    add $4, sp

    push $buf           # destination buffer
    push $buf+6         # file path
    jsr pc, _fs_stat_path
    add $4, sp
    pop r5

    mfpi 6(r5)          # destination buffer => r1
    pop r1

    mov $buf, r2        # source buffer => r2

    mov $3, r3          # size of stat_t struct = 3 words
1$:
    push (r2)+          # copy 1 word at a time
    mtpi (r1)+
    sob r3, 1$

    jmp ret


# No arguments
trap.mkfs:
    jsr pc, _fs_mkfs
    jmp ret


# r5 points to user-space stack:
#     - child pid
#     - return address for call to syscall stub
# r5 -> old r5
trap.wait:
    mfpi 4(r5)
    jsr pc, _proc_wait
    add $2, sp
    jmp ret


# r5 points to user-space stack:
#     - path string
#     - return address for call to syscall stub
# r5 -> old r5
trap.chdir:
    mfpi 4(r5)          # target dir name
    push $buf
    jsr pc, readbuf
    add $4, sp

    push $buf
    jsr pc, _proc_chdir
    add $2, sp
    jmp ret


# r5 points to user-space stack:
#     - buffer size
#     - destination buf
#     - return address for call to syscall stub
# r5 -> old r5
trap.getcwd:
    mfpi 6(r5)          # buffer size
    push $buf
    jsr pc, _proc_getcwd
    add $4, sp

    tst r0
    ble 1$             # failed or nothing to copy? return

    push r0             # r0 has the size of the string
    mfpi 4(r5)          # user space destination
    push $buf
    jsr pc, writebuf
    add $4, sp

1$:
    pop r0
    jmp ret


# r5 points to user-space stack:
#     - read fd dest ptr
#     - write fd dest ptr
#     - return address for call to syscall stub
# r5 -> old r5
trap.pipe:
    sub $4, sp   # clear some space to store out params
    mov sp, r1
    push r1      # read fd on top of stack
    add $2, r1
    push r1      # write fd

    jsr pc, _io_pipe
    add $4, sp

    mfpi 6(r5)  # read fd
    pop r1

    mfpi 4(r5)  # write fd
    pop r2

    mtpi (r1)
    mtpi (r2)

    jmp ret

# r5 points to user-space stack:
#     - target fd
#     - original fd
#     - return address for call to syscall stub
# r5 -> old r5
trap.dup2:
    mfpi 6(r5)
    mfpi 4(r5)
    jsr pc, _io_dup2
    add $4, sp
    jmp ret
