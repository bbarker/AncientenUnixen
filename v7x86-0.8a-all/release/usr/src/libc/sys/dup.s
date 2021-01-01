// V7/x86 source code: see www.nordier.com/v7x86 for details.
// Copyright (c) 1999 Robert Nordier.  All rights reserved.

		.globl _dup, _dup2, cerror
_dup2:		orl $0100,0x4(esp)

_dup:		mov $41,eax
                int $0x30
                jc 1f
                ret

1:            	jmp cerror
