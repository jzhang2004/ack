.define	.strhp
.sect .text
.sect .rom
.sect .data
.sect .bss

	.sect .text
.strhp:
	move.l	4(sp), d1	! new heap pointer
	cmp.l	.limhp, d1
	blt	1f
	add.l	#0x400, d1
	and.l	#~0x3ff, d1
	move.l	d1, .limhp
	move.l	d1, -(sp)
	jsr	_brk		! allocate 1K bytes of extra storage
	add.l	#4, sp
	bcs	2f
1:
	move.l	4(sp), .reghp	! store new value of heap pointer
	move.l	(sp)+,a0
	move.l	a0,(sp)
	rts
2:
	move.l	#EHEAP, -(sp)
	jmp	.fatal
.align 2
