// UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details.
// Changes: Copyright (c) 1999 Robert Nordier. All rights reserved.

// C library -- fork

// pid = fork();
//
// pid == 0 in child process; pid == -1 means error return
// in child, parents id is in par_uid if needed

.globl	_fork, _par_uid
.globl	cerror
.set	.fork,2

_fork:
	mov	$.fork,eax
	int	$0x30
	jmp	1f
	jnc	2f
	jmp	cerror
1:
	mov	eax,_par_uid
	xor	eax,eax
2:
	ret

.comm	_par_uid,4
