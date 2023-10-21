.include "macros.s"

.globl readbuf
.globl writebuf
    
.text
.even

# Read from user-space buffer (top of stack)
# returns: address after the last byte copied (r0)
readbuf:
    mov 2(sp), r0       # buffer to copy into
    mov 4(sp), r1       # user-space pointer

    push r2

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
    pop r2
    rts pc


# Copy data from kernel space to user space
writebuf:
    mov 2(sp), r2       # kernel-space address to copy from
    mov 4(sp), r1       # user-space address to copy to
    mov 6(sp), r0       # number of bytes to read

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

    rts pc

