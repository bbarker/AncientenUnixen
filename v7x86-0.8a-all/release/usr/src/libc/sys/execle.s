// UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details.
// Changes: Copyright (c) 1999 Robert Nordier. All rights reserved.

// C library -- execle

// execle(file, arg1, arg2, ... , 0, env);
//

.globl	_execle
.globl	_execve

_execle:
	push	ebp
	mov	esp,ebp
	push	8(ebp)
	lea	12(ebp),eax
	push	eax
1:
	mov	(eax),ecx
	add	$4,eax
	test	ecx,ecx
	jnz	1b
	push	(eax)
	call	_execve
	leave
	ret
