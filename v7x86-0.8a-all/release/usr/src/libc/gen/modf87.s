// V7/x86 source code: see www.nordier.com/v7x86 for details.
// Copyright (c) 1999 Robert Nordier.  All rights reserved.

.globl _modf
_modf:
	push	ebp
	mov	esp,ebp
	mov	0x8(ebp),edx
	mov	0xc(ebp),ebx
	mov	$0x80000000,eax
	and	ebx,eax
	push	eax
	push	$0
	mov	ebx,eax
	rol	$12,eax
	and	$0x7ff,eax
	sub	$0x3ff,eax
	jb	3f
	cmp	$51,eax
	ja	2f
	cmpb	$32,al
	jb	1f
	mov	edx,ebx
	xor	edx,edx
	subb	$32,al
1:
	movb	al,cl
	shld	cl,edx,ebx
	shl	cl,edx
	and	$0xfffff,ebx
	jnz	1f
	test	edx,edx
	jz	2f
1:
	mov	$0x3ff,eax
1:
	dec	eax
	add	edx,edx
	adc	ebx,ebx
	test	$0x100000,ebx
	jz	1b
	xor	$0x100000,ebx
	ror	$12,eax
	or	eax,ebx
	or	-0x4(ebp),ebx
	mov	edx,-0x8(ebp)
	mov	ebx,-0x4(ebp)
2:
	fldl	-0x8(ebp)
	fldl	0x8(ebp)
	fsub	st(1),st
2:
	mov	0x10(ebp),eax
	fstpl	(eax)
	fstpl	(esp)
	pop	eax
	pop	edx
	leave
	ret
3:
	fldl	0x8(ebp)
	fldl	-0x8(ebp)
	jmp	2b
