// UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details.
// Changes: Copyright (c) 1999 Robert Nordier. All rights reserved.

// C library -- exit

// exit(code)
// code is returned to system

.globl	_exit
.globl	__cleanup
.set exit,1

_exit:
	call	__cleanup
	mov	$exit,eax
	int	$0x30
