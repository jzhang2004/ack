.sect .text; .sect .rom; .sect .data; .sect .bss
.sect .text
link = 9
.define	__link

__link:
	.data2	0x0000
	chmk	$link
	bcc 	1f
	jmp 	cerror
1:
	clrl	r0
	ret
