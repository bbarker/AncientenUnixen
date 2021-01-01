// UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details.
// Changes: Copyright (c) 1999 Robert Nordier. All rights reserved.

// C runtime startoff including monitoring

.set	cbufs,300
.set	exit,1

.globl	_monitor
.globl	_sbrk
.globl	_main
.globl	_exit
.globl	_environ
.globl	_etext
.globl	__cleanup
.globl	countbase


start:
	cld
	mov	(esp),ecx
	lea	4(esp),ebx
	lea	4(ebx,ecx,4),eax
	mov	eax,(esp)
	mov	eax,_environ
	push	ebx
	push	ecx
	mov	$_etext,ebx
	sub	$eprol,ebx
	add	$7,ebx
	shr	$3,ebx
	push	$cbufs
	shl	ebx
	add	$8*cbufs+12,ebx
	push	ebx
	push	ebx
	call	_sbrk
	pop	ecx
	cmp	$-1,eax
	je	9f
	push	eax
	add	$12,eax
	mov	eax,countbase
	push	$_etext
	push	$eprol
	call	_monitor
	add	$20,esp
	call	_main
	add	$8,esp
	mov	eax,(esp)
	call	_exit

9:
	push	$9f-8f
	push	$8f
	push	$2
	call	_write
	add	$12,esp

.data; 8:.asciz "No space for monitor buffer\n"; 9:.align 4; .text

_exit:
	push	ebp
	mov	esp,ebp
	call	__cleanup
	push	$0
	call	_monitor
	pop	ecx
	pop	ebp
	mov	$exit,eax
	int	$0x30
eprol:

.comm	_environ,4
.comm	countbase,4
