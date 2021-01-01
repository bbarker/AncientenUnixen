// UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details.
// Changes: Copyright (c) 1999 Robert Nordier. All rights reserved.

// C library-- floating output

.globl	pfloat
.globl	pscien
.globl	pgen
.globl	fltused

.globl	_ecvt
.globl	_fcvt
.globl	_gcvt

fltused:		// force loading

pgen:
	push	ebx
	push	edi
	push	eax
	test	ecx,ecx
	jne	1f
	movl	$6,(esp)
1:
	fldl	(ebx)
	sub	$8,esp
	fstpl	(esp)
	call	_gcvt
	add	$8+4+4,esp
	xor	al,al
1:
	scasb
	jne	1b
	dec	edi
	pop	ebx
	add	$8,ebx
	ret

pfloat:
	push	ebx
	push	$sign
	push	$decpt
	test	ecx,ecx
	jne	1f
	mov	$6,eax
1:
	push	eax
	mov	eax,ndigit
	fldl	(ebx)
	sub	$8,esp
	fstpl	(esp)
	call	_fcvt
	add	$8+4+4+4,esp
	push	esi
	mov	eax,esi
	cmpb	$0,sign
	je	1f
	mov	$'-',al
	stosb
1:
	mov	decpt,ecx
	test	ecx,ecx
	jg	2f
	mov	$'0',al
	stosb
	jmp	1f
2:
	movsb
	loop	2b
1:
	mov	ndigit,edx
	test	edx,edx
	je	1f
	mov	$'.',al
	stosb
1:
	mov	decpt,ecx
	neg	ecx
	jle	1f
	mov	$'0',al
2:
	dec	edx
	jl	1f
	stosb
	loop	2b
1:
	test	edx,edx
	jle	2f
1:
	movsb
	dec	edx
	jnz	1b
2:
	pop	esi
	pop	ebx
	add	$8,ebx
	ret

pscien:
	push	ebx
	push	$sign
	push	$decpt
	push	eax
	mov	eax,ndigit
	test	ecx,ecx
	jne	1f
	movl	$6,(esp)
1:
	fldl	(ebx)
	sub	$8,esp
	fstpl	(esp)
	call	_ecvt
	add	$8+4+4+4,esp
	push	esi
	mov	eax,esi
	cmpb	$0,sign
	je	1f
	mov	$'-',al
	stosb
1:
	cmpb	$'0',(edi)
	jne	1f
	incl	decpt
1:
	movsb
	mov	$'.',al
	stosb
	mov	ndigit,ecx
	dec	ecx
	jle	1f
2:
	movsb
	loop	2b
1:
	mov	$'e',al
	stosb
	mov	decpt,edx
	dec	edx			// sub $1,ecx
	jge	1f
	mov	$'-',al
	neg	edx
	jmp	2f
1:
	mov	$'+',al
2:
	stosb
	xor	eax,eax
	xchg	edx,eax
	mov	$10,ecx
	div	ecx
	add	$'0',al
	stosb
	mov	edx,eax
	add	$'0',al
	stosb
	pop	esi
	pop	ebx
	add	$8,ebx
	ret

.lcomm	sign,4
.lcomm	ndigit,4
.lcomm	decpt,4
