.define _close
.extern _close
.sect .text
.sect .rom
.sect .data
.sect .bss
.sect .text
_close:		move.l #0x6,d0
		move.l 4(sp),a0
		jmp callc
