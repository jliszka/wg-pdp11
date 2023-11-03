.title	sh.c
	.enabl	lsb,reg

	.psect	code,i,ro,con
	.psect	rodata,d,ro,con
1$:
	.asciz	"$ "
2$:
	.asciz	"cd"
3$:
	.asciz	"exec failed: "
4$:
	.asciz	") "
	.psect	code,i,ro,con
	.even
	.globl	main
main:
	setd
	seti
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	r4,-(sp)
	jsr	pc,__main
4$:
	mov	#40,-(sp)
	mov	#path,-(sp)
	jsr	pc,getcwd
	mov	#path,-(sp)
	mov	#print,r2
	jsr	pc,(r2)
	mov	#1$,-(sp)
	jsr	pc,(r2)
	mov	#1,-(sp)
	jsr	pc,fflush
	mov	#cmd,-(sp)
	mov	#400,-(sp)
	jsr	pc,input
	mov	#20,-(sp)
	mov	#argv,-(sp)
	movb	#40,-(sp)
	mov	#cmd,-(sp)
	jsr	pc,strntok
	mov	r0,r3
	add	#26,sp
	tst	r0
	beq	4$
	cmp	r0,#2
	bne	5$
	mov	#3,-(sp)
	mov	#2$,-(sp)
	mov	argv,-(sp)
	jsr	pc,strncmp
	add	#6,sp
	tst	r0
	bne	5$
	mov	argv+2,-(sp)
	jsr	pc,chdir
	add	#2,sp
	br	4$
5$:
	jsr	pc,fork
	mov	#itoa,r4
	tst	r0
	beq	11$
	mov	r0,-(sp)
	jsr	pc,wait
	mov	#buf,-(sp)
	mov	r0,-(sp)
	mov	#12,-(sp)
	jsr	pc,(r4)
	mov	r0,-(sp)
	jsr	pc,(r2)
	mov	#4$,-(sp)
	jsr	pc,(r2)
	add	#14,sp
	br	4$
11$:
	mov	#argv,-(sp)
	mov	r3,-(sp)
	jsr	pc,exec
	mov	r0,r3
	mov	#3$,-(sp)
	jsr	pc,(r2)
	mov	#buf,-(sp)
	mov	r3,-(sp)
	mov	#12,-(sp)
	jsr	pc,(r4)
	mov	r0,-(sp)
	jsr	pc,println
	add	#16,sp
	mov	r3,r0
	mov	(sp)+,r4
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc
	.globl	argv
	.psect	data,d,rw,con
	.even
argv:
	.blkb	40
	.globl	cmd
cmd:
	.blkb	400
	.globl	buf
buf:
	.blkb	100
	.globl	path
path:
	.asciz	"aaa"
	.blkb	34
	.globl	println
	.globl	exec
	.globl	wait
	.globl	itoa
	.globl	fork
	.globl	chdir
	.globl	strncmp
	.globl	strntok
	.globl	input
	.globl	fflush
	.globl	print
	.globl	getcwd
	.end
