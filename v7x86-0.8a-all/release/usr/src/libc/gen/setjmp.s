// UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details.
// Changes: Copyright (c) 1999 Robert Nordier. All rights reserved.

// C library -- setjmp, longjmp

//	longjmp(a,v)
// will generate a "return(v)" from
// the last call to
//	setjmp(a)
// by restoring sp, r5, pc from `a'
// and doing a return.

.globl	_setjmp
.globl	_longjmp

_setjmp:
	mov	0x4(esp),edx
	mov	(esp),ecx
	mov	ecx,(edx)
	mov	esp,0x4(edx)
	mov	ebp,0x8(edx)
	mov	edi,0xc(edx)
	mov	esi,0x10(edx)
	mov	ebx,0x14(edx)	// XXX could be dropped
	// XXX fnstcw 0x1c(edx)
	xor	eax,eax
	ret

_longjmp:
	mov 0x4(esp),edx
	mov 0x8(esp),eax
	mov 0x14(edx),ebx	// XXX could be dropped
	mov 0x10(edx),esi
	mov 0xc(edx),edi
	mov 0x8(edx),ebp
	mov 0x4(edx),esp
	mov (edx),ecx
	mov ecx,(esp)
	// XXX fninit
	// XXX fldcw 0x1c(edx)
	test eax,eax
	jnz 1f
	inc eax
1:	ret
