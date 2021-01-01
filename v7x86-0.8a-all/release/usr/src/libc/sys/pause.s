// V7/x86 source code: see www.nordier.com/v7x86 for details.
// Copyright (c) 1999 Robert Nordier.  All rights reserved.

                .globl _pause
_pause:		mov $29,eax
                int $0x30
                ret
