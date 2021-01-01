// V7/x86 source code: see www.nordier.com/v7x86 for details.
// Copyright (c) 1999 Robert Nordier.  All rights reserved.

                .globl _ldexp
_ldexp: 
		fildl 0xc(esp)
		fldl 0x4(esp)
		fscale
		fstp st(1)
		ret
