
.include "macros.s"

.globl _userexec
.globl _kret

.text
.even

# "Reverse" trap to jump to user mode program. Args:
#   - user mode stack pointer
_userexec:
    mov 2(sp), r0       # user mode stack pointer

    push r2
    push r3
    push r4
    push r5

    push $0140000
    push $020000
    rti                 # simultaneously set mode and "jump"


# Return into a different context by setting the kernel stack pointer
#   - 6(sp) address to place old ksp
#   - 4(sp) kernel stack page
#   - 2(sp) kernel stack pointer
#   - return address
_kret:
    # kernel stack page: unibus map low word
    mov 4(sp), r1   # kernel stack page
    ash $13, r1
    mov r1, @$0170230

    # kernel stack page: unibus map high word
    mov 4(sp), r1
    ash $-3, r1
    mov r1, @$0170232


    # r0: address of old stack pointer struct field
    mov 6(sp), r0

    # r1: new kernel stack pointer
    mov 2(sp), r1

    push r2

    # r2: kernel stack page: CPU page table
    mov 6(sp), r2       # used to be 4(sp)
    ash $7, r2

    push r3
    push r4
    push r5

    mov sp, (r0)        # save old ksp
    mov r2, @$0172354   # swap page table
    mov r1, sp          # restore new ksp

    pop r5
    pop r4
    pop r3
    pop r2

    rts pc

    
    
