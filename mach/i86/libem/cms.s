.sect .text; .sect .rom; .sect .data; .sect .bss
.define .cms

	! #bytes in cx
	.sect .text
.cms:
	pop     bx              ! return address
	mov     dx,sp
	push	si
	push	di
	mov     si,dx
	add     dx,cx
	mov     di,dx
	add     dx,cx
	sar     cx,1
	repe cmps
	je      1f
	inc     cx
1:
	pop	di
	pop	si
	mov     sp,dx
	jmp     bx
