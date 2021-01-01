// V7/x86 source code: see www.nordier.com/v7x86 for details.
// Copyright (c) 1999 Robert Nordier.  All rights reserved.

.set NSIG,17

.set .signal,48
.globl  _signal, cerror

_signal:	mov 0x4(esp),edx
		cmp $NSIG,edx
		jae 2f
		mov 0x8(esp),eax
		shl $2,edx
		mov dvect(edx),ecx
		mov eax,dvect(edx)
		cmp $1,eax
		jbe 1f
		add $tvect,edx
		mov edx,0x8(esp)
1:		mov $.signal,eax
		int $0x30
		jc 3f
		cmp $1,eax
		jne 1f
		mov eax,ecx
1:		mov ecx,eax
		ret
		
2:		mov $22,eax			// EINVAL
3:		jmp cerror

tvect:		push $0x00
		jmp 1f
		push $0x01
		jmp 1f
		push $0x02
		jmp 1f
		push $0x03
		jmp 1f
		push $0x04
		jmp 1f
		push $0x05
		jmp 1f
		push $0x06
		jmp 1f
		push $0x07
		jmp 1f
		push $0x08
		jmp 1f
		push $0x09
		jmp 1f
		push $0x0a
		jmp 1f
		push $0x0b
		jmp 1f
		push $0x0c
		jmp 1f
		push $0x0d
		jmp 1f
		push $0x0e
		jmp 1f
		push $0x0f
		jmp 1f
		push $0x10

1:		pushf
		push eax
		push ecx
		push edx
		push ebx
		mov 0x14(esp),eax
		push eax
		shl $2,eax
		call *dvect(eax)
		add $0x4,esp
		pop ebx
		pop edx
		pop ecx
		pop eax
		popf
		lea 4(esp),esp
		ret

		.lcomm dvect,NSIG*4
