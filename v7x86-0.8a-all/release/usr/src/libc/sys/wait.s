// V7/x86 source code: see www.nordier.com/v7x86 for details.
// Copyright (c) 1999 Robert Nordier.  All rights reserved.

		.globl _wait, cerror
_wait:		mov $7,eax
                int $0x30
                jc 2f
		mov 0x4(esp),ecx
		jecxz 1f
		mov edx,(ecx)
1:              ret

2:            	jmp cerror
