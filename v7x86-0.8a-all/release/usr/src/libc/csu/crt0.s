// V7/x86 source code: see www.nordier.com/v7x86 for details.
// Copyright (c) 2007 Robert Nordier.  All rights reserved.

.globl	start, _environ, _exit, __exit

start:
	cld
	mov	(esp),ecx
	lea	4(esp),ebx
	lea	4(ebx,ecx,4),eax
	mov	eax,_environ
	push	eax
	push	ebx
	push	ecx
	call	_main
	add	$12,esp
	push	eax
	call	_exit
	call	__exit

.comm	_environ,4
