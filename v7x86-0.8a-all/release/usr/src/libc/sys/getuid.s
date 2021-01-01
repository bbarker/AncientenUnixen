// V7/x86 source code: see www.nordier.com/v7x86 for details.
// Copyright (c) 1999 Robert Nordier.  All rights reserved.

                .globl _getuid
_getuid:	mov $24,eax
                int $0x30
                ret
