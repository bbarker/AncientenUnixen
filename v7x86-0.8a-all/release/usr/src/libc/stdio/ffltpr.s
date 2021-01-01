// UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details.
// Changes: Copyright (c) 1999 Robert Nordier. All rights reserved.

// C library-- fake floating output

.globl	pfloat
.globl	pscien
.globl	pgen

pfloat:
pscien:
pgen:
	add	$0x8,ebx
	movb	$'?',al
	stosb
	ret
