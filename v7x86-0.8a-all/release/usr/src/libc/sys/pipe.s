// V7/x86 source code: see www.nordier.com/v7x86 for details.
// Copyright (c) 1999 Robert Nordier.  All rights reserved.

		.globl _pipe, cerror
_pipe:		mov $42,eax
                int $0x30
                jc 1f
		mov 0x4(esp),ecx
		mov eax,(ecx)
		mov edx,0x4(ecx)
		xor eax,eax
                ret

1:            	jmp cerror
