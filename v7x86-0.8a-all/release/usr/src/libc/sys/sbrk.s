// V7/x86 source code: see www.nordier.com/v7x86 for details.
// Copyright (c) 1999 Robert Nordier.  All rights reserved.

.globl _sbrk, _brk
.globl _end, cerror
.set .break,17

_sbrk:		mov 0x4(esp),ecx
		jecxz 1f
		mov nd,eax
		add eax,0x4(esp)
		mov $.break,eax
		int $0x30
		jc 2f
1:		mov nd,eax
		add ecx,nd
		ret

2:            	jmp cerror

_brk:		mov $.break,eax
                int $0x30
                jc 2b
		mov 0x4(esp),eax
		mov eax,nd
		xor eax,eax
		ret

		.data
nd:		.long _end
