// V7/x86 source code: see www.nordier.com/v7x86 for details.
// Copyright (c) 1999 Robert Nordier.  All rights reserved.

		.globl _write, cerror
_write:		mov $4,eax
                int $0x30
                jc 1f
                ret

1:            	jmp cerror
