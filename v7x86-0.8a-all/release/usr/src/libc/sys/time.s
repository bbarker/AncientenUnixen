// V7/x86 source code: see www.nordier.com/v7x86 for details.
// Copyright (c) 1999 Robert Nordier.  All rights reserved.

		.globl _time
_time:		mov $13,eax
                int $0x30
		mov 0x4(esp),ecx
		jecxz 1f
		mov eax,(ecx)
1:		ret

		.globl _ftime, cerror
_ftime:		mov $35,eax
                int $0x30
                jc 1f
                ret

1:            	jmp cerror
