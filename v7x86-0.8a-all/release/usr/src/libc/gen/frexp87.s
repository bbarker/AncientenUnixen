// V7/x86 source code: see www.nordier.com/v7x86 for details.
// Copyright (c) 1999 Robert Nordier.  All rights reserved.

.globl _frexp
_frexp:
	fldl 	0x4(esp)
	ftst
	fnstsw	ax
	sahf
	movl 	0xc(esp),eax
	jnz 	1f
	movl 	$0,(eax)
	ret
1:		
	fxtract
	fxch
	fistpl 	(eax)
	incl 	(eax)
	fld1
	fchs
	fxch
	fscale
	fstp 	st(1)
	ret
