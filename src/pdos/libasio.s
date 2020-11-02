.title	libasio.c
	.enabl	lsb,reg

	.psect	code,i,ro,con
	.even
	.globl	io_init
io_init:
	mov	#outbuf,outend
	mov	#outbuf,outptr
	mov	#inbuf,inptr
	mov	#-214,r1
	mov	(r1),r0
	bis	#100,r0
	mov	r0,(r1)
	mov	#-220,r1
	mov	(r1),r0
	bic	#100,r0
	mov	r0,(r1)
	jsr	pc,isrinit
	rts	pc
	.even
	.globl	write
write:
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	6(sp),r1
4$:
	movb	(r1)+,r0
	tstb	r0
	bne	5$
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc
5$:
	mov	outend,r2
	mov	r2,r3
	inc	r3
	mov	r3,outend
	movb	r0,(r2)
	br	4$
	.even
	.globl	read
read:
	mov	r2,-(sp)
	mov	4(sp),r0
7$:
	mov	inptr,r1
	cmpb	-1(r1),#15
	bne	8$
	mov	#inbuf,r2
9$:
	movb	(r2)+,r1
	cmpb	r1,#15
	bne	10$
	clrb	(r0)
	mov	#inbuf,inptr
	mov	(sp)+,r2
	rts	pc
8$:
;# 74 "libasio.c" 1
	wait
;# 0 "" 2
	br	7$
10$:
	movb	r1,(r0)+
	br	9$
	.even
	.globl	flush
flush:
	mov	#-214,r1
	mov	(r1),r0
	bis	#100,r0
	mov	r0,(r1)
12$:
	cmp	outend,#outbuf
	bne	13$
	rts	pc
13$:
;# 90 "libasio.c" 1
	wait
;# 0 "" 2
	br	12$
	.psect	rodata,d,ro,con
1$:
	.asciz	<15><12>
	.psect	code,i,ro,con
	.even
	.globl	writeln
writeln:
	mov	r2,-(sp)
	mov	4(sp),-(sp)
	mov	#write,r2
	jsr	pc,(r2)
	mov	#1$,-(sp)
	jsr	pc,(r2)
	jsr	pc,flush
	add	#4,sp
	mov	(sp)+,r2
	rts	pc
	.even
	.globl	kb_handler
kb_handler:
	mov	r2,-(sp)
	movb	@#-216,r1
	movb	r1,@#-212
	mov	inptr,r0
	mov	r0,r2
	inc	r2
	mov	r2,inptr
	movb	r1,(r0)
	mov	(sp)+,r2
	rts	pc
	.even
	.globl	tt_handler
tt_handler:
	mov	outptr,r0
	cmp	r0,outend
	bne	17$
	cmp	r0,#outbuf
	beq	16$
	mov	#outbuf,outptr
	mov	#outbuf,outend
	mov	#-214,r1
	mov	(r1),r0
	bic	#100,r0
	mov	r0,(r1)
	rts	pc
17$:
	mov	r0,r1
	inc	r1
	mov	r1,outptr
	movb	(r0),@#-212
16$:
	rts	pc
	.even
	.globl	ptr_has_next
ptr_has_next:
	mov	ptr_status,r0
	cmp	r0,#1
	beq	22$
	cmp	r0,#-1
	beq	26$
	mov	#-230,r1
	mov	(r1),r0
	bis	#1,r0
	mov	r0,(r1)
24$:
	mov	#-230,r1
	mov	(r1),r0
	bic	#-201,r0
	beq	25$
	movb	@#-226,r0
	movb	r0,ptr_char
	mov	#1,ptr_status
	mov	#1,r0
	rts	pc
25$:
	mov	(r1),r0
	tst	r0
	bge	24$
	mov	#-1,ptr_status
26$:
	clr	r0
22$:
	rts	pc
	.even
	.globl	ptr_next
ptr_next:
	clr	ptr_status
	movb	ptr_char,r0
	rts	pc
	.even
	.globl	ptr_read
ptr_read:
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	6(sp),r2
	clr	r0
	mov	r2,r1
	inc	r1
	tst	r2
	bge	33$
	mov	#1,r1
33$:
	dec	r1
	beq	40$
	jmp	36$
40$:
	br	32$
36$:
	mov	#-230,r3
	mov	(r3),r2
	bis	#1,r2
	mov	r2,(r3)
34$:
	mov	#-230,r3
	mov	(r3),r2
	bic	#-201,r2
	beq	35$
	movb	@#-226,r3
	mov	10(sp),r2
	add	r0,r2
	movb	r3,(r2)
	inc	r0
	br	33$
35$:
	mov	(r3),r2
	tst	r2
	bge	34$
32$:
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc
	.globl	ptr_char
	.psect	data,d,rw,con
ptr_char:
	.blkb	1
	.globl	ptr_status
	.even
ptr_status:
	.blkb	2
	.globl	inptr
	.even
inptr:
	.word	inbuf
	.globl	inbuf
inbuf:
	.blkb	100
	.globl	outptr
	.even
outptr:
	.word	outbuf
	.globl	outend
	.even
outend:
	.word	outbuf
	.globl	outbuf
outbuf:
	.blkb	100
	.globl	isrinit
	.end
