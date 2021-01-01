// UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details.
// Changes: Copyright (c) 1999 Robert Nordier. All rights reserved.

//
// ldfps(number);

.globl	_ldfps
_ldfps:
	push	ebp
	mov	esp,ebp
	fldl	8(ebp)
	pop	ebp
	ret
