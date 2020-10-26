	.text
	.even
	.globl	_writechr
_writechr:
	mov	r5,-(sp)
	mov	sp,r5
	add	$-04,sp
	mov	$-0214,-02(r5)
	mov	$-0212,-04(r5)
	nop
L_2:
	mov	@-02(r5),r0
	bic	$-0201,r0
	tst	r0
	beq	L_2
	movb	04(r5),r0
	movb	r0,@-04(r5)
	nop
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc
	.even
	.globl	_writestr
_writestr:
	mov	r5,-(sp)
	mov	sp,r5
	add	$-02,sp
	mov	04(r5),-02(r5)
	br	L_4
L_5:
	mov	-02(r5),r0
	mov	r0,r1
	inc	r1
	mov	r1,-02(r5)
	movb	(r0),r0
	movb	r0,-(sp)
	jsr	pc,_writechr
	add	$02,sp
L_4:
	movb	@-02(r5),r0
	tstb	r0
	bne	L_5
	nop
	nop
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc
	.data
LC_0:
	.byte 0150,0145,0154,0154,0157,0
LC_1:
	.byte 0167,060,0162,0154,0144,0
	.text
	.even
	.globl	_hello
_hello:
	mov	r5,-(sp)
	mov	sp,r5
	movb	$015,-(sp)
	jsr	pc,_writechr
	add	$02,sp
	movb	$012,-(sp)
	jsr	pc,_writechr
	add	$02,sp
	movb	$015,-(sp)
	jsr	pc,_writechr
	add	$02,sp
	movb	$012,-(sp)
	jsr	pc,_writechr
	add	$02,sp
	mov	$LC_0,-(sp)
	jsr	pc,_writestr
	add	$02,sp
	movb	$015,-(sp)
	jsr	pc,_writechr
	add	$02,sp
	movb	$012,-(sp)
	jsr	pc,_writechr
	add	$02,sp
	mov	$LC_1,-(sp)
	jsr	pc,_writestr
	add	$02,sp
	movb	$015,-(sp)
	jsr	pc,_writechr
	add	$02,sp
	movb	$012,-(sp)
	jsr	pc,_writechr
	add	$02,sp
	nop
	mov	(sp)+,r5
	rts	pc
	.even
	.globl	_main
_main:
	mov	r5,-(sp)
	mov	sp,r5
	jsr	pc,___main
	jsr	pc,_hello
	clr	r0
	mov	(sp)+,r5
	rts	pc
