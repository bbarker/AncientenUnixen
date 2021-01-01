// UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details.
// Changes: Copyright (c) 1999 Robert Nordier. All rights reserved.

// count subroutine calls during profiling

.globl	mcount, countbase
.comm	countbase,4

mcount:
	mov	(eax),edx
	test	edx,edx
	jne	1f
	mov	countbase,edx
	test	edx,edx
	je	2f
	addl	$8,countbase
	mov	(esp),ecx
	mov	ecx,(edx)
	add	$4,edx
	mov	edx,(eax)
1:
	incl	(edx)
2:
	ret
