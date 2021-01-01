// UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details.
// Changes: Copyright (c) 1999 Robert Nordier. All rights reserved.

// C library -- conversions

.set width,-16
.set rjust,-20
.set ndfnd,-24
.set ndigit,-28
.set zfill,-32

.globl	__doprnt

.globl	pfloat
.globl	pscien
.globl	pgen

.globl	__strout

__doprnt:
	push	ebp
	mov	esp,ebp
	push	ebx			// XXX
	push	esi
	push	edi
	sub	$128+20,esp
	mov	0x08(ebp),esi
	mov	0x0c(ebp),ebx
	cld
loop:
	mov	esp,edi
2:
	lodsb
	testb	al,al
	jz	2f
	cmpb	$'%',al
	je	2f
	stosb
	jmp	2b
2:
	cmp	edi,esp
	je	2f
	mov	esp,ecx
	push 	ebx
	push	eax
	pushl	0x10(ebp)
	push	$0
	push	edi
	sub	ecx,(esp)
	push	ecx
	call	__strout
	add	$0x10,esp
	pop	eax
	pop	ebx
2:
	testb	al,al
	jnz	2f
	add	$128+20,esp
	pop	edi
	pop	esi
	pop	ebx
	pop	ebp
	ret

2:
	mov	esp,edi
2:
	movb	$0,rjust(ebp)
	movl	$0,ndigit(ebp)
	movl	$' ',zfill(ebp)
	cmpb	$'-',(esi)
	jne	2f
	incb	rjust(ebp)
	inc	esi
2:
	cmpb	$'0',(esi)
	jne	2f
	movb	$'0',zfill(ebp)
2:
	call	gnum
	mov	edx,width(ebp)
	movl	$0,ndfnd(ebp)
	cmpb	$'.',al
	jne	1f
	call	gnum
	mov	edx,ndigit(ebp)
1:
	push	edi
	mov	$swtab1,edi
	mov	$16,ecx
	repne
	scasb
	pop	edi
	jne	2f
	jmp	*swtab(,ecx,4)
2:
	stosb
	jmp	prbuf
	.data
swtab:
	.long	decimal 		// 'd'
	.long	octal			// 'o'
	.long	hex			// 'x'
	.long	float			// 'f'
	.long	scien			// 'e'
	.long	general 		// 'g'
	.long	charac			// 'c'
	.long	string			// 's'
	.long	longorunsg		// 'l'
	.long	longorunsg		// 'L'
	.long	unsigned		// 'u'
	.long	remote			// 'r'
	.long	ldec			// 'D'
	.long	loct			// 'O'
	.long	lhex			// 'X'
	.long	lunsigned		// 'U'
swtab1:
	.ascii	"UXODruLlscgefxod"
	.text

general:
	mov	ndigit(ebp),eax
	mov	ndfnd(ebp),ecx
	call	pgen
	jmp	prbuf

loct:
octal:
	mov	$8,ecx
	jmp	compute

lhex:
hex:
	mov	$16,ecx
	jmp	compute

ldec:
decimal:
	mov	$10,ecx
	mov	(ebx),edx
	test	edx,edx
	jns	compute1
	neg	edx
	movb	$'-',al
	stosb
	jmp	compute1

longorunsg:
	lodsb
	cmpb	$'o',al
	je	loct
	cmpb	$'x',al
	je	lhex
	cmpb	$'d',al
	je	ldec
	cmpb	$'u',al
	je	lunsigned
	dec	esi

lunsigned:
unsigned:
	mov	$10,ecx

compute:
	mov	(ebx),edx

compute1:
	add	$0x4,ebx
	test	edx,edx
	jz	1f
	cmpl	$0,ndigit(ebp)
	je	1f
	movb	$'0',al
	stosb
1:
	call	1f
	jmp	prbuf

1:
	mov	edx,eax
2:
	xor	edx,edx
	div	ecx
	push	edx
	test	eax,eax
	jz	1f
	call	2b
1:
	pop	eax
	cmpb	$10,al
	sbbb	$0x69,al
	das
	orb	$0x20,al
	stosb
	ret

charac:
	movb	$' ',zfill(ebp)
	movb	(ebx),al
	add	$0x4,ebx
	testb	al,al
	jz	prbuf
	stosb
	jmp	prbuf

string:
	movb	$' ',zfill(ebp)
	mov	ndigit(ebp),ecx
	mov	(ebx),edi
	test	edi,edi
	jnz	1f
	mov	$nulstr,edi
	mov	edi,(ebx)
1:
	cmpb	$0,(edi)
	je	1f
	inc	edi
	loop	1b
1:
	mov	(ebx),ecx
	add	$0x4,ebx
	jmp	prstr

float:
	mov	ndigit(ebp),eax
	mov	ndfnd(ebp),ecx
	call	pfloat
	jmp	prbuf

scien:
	mov	ndigit(ebp),eax
	inc	eax
	cmpl	$0,ndfnd(ebp)
	jne	1f
	mov	$7,eax
1:
	call	pscien
	jmp	prbuf

remote:
	mov	(ebx),ebx
	mov	(ebx),esi
	jmp	loop

prbuf:
	mov	esp,ecx

prstr:
	sub	ecx,edi
	mov	width(ebp),edx
	sub	edi,edx
	jge	1f
	xor	edx,edx
1:
	cmpb	$0,rjust(ebp)
	jne	1f
	neg	edx
1:
	push	ebx
	pushl	zfill(ebp)
	pushl	0x10(ebp)
	push	edx
	push	edi
	push	ecx
	call	__strout
	add	$0x14,esp
	pop	ebx
	jmp	loop

gnum:
	xor	edx,edx
	mov	edx,ndfnd(ebp)
1:
	xor	eax,eax
	lodsb
	subb	$'0',al
	cmpb	$'*'-'0',al
	jne	2f
	mov	(ebx),eax
	add	$0x4,ebx
	jmp	3f
2:
	cmpb	$9,al
	ja	1f

3:
	incl	ndfnd(ebp)
	mov	edx,ecx
	shl	$0x2,edx
	add	ecx,edx
	shl	edx
	add	eax,edx
	jmp	1b
1:
	addb	$'0',al
	ret

.data
nulstr:
	.ascii	"(null)\0"
