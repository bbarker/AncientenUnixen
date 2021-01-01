// V7/x86 source code: see www.nordier.com/v7x86 for details.
// Copyright (c) 2006 Robert Nordier.  All rights reserved.

	.code16

	.set	LOAD,0x7c00		// Load address
	.set	EXEC,0x600		// Execution address
	.set	PT_OFF,0x1be		// Partition table offset
	.set	MAGIC,0xaa55		// Magic: bootable
	.set	BSIZE,0x200		// Block size

start:
	cld
	xor	ax,ax
	mov	ax,es
	mov	ax,ds
	mov	ax,ss
	mov	$LOAD,sp
	mov	$main-EXEC+LOAD,si	// Relocate to EXEC
	mov	$main,di
	mov	$BSIZE-[main-start],cx
	rep; movsb
	jmp	main-LOAD+EXEC

main:
	xor	si,si			// Find active partition
	mov	$partab,bx
	mov	$0x4,cl
1:
	cmp	ch,(bx)
	je	2f
	jg	err_pt
	test	si,si
	jnz	err_pt
	mov	bx,si
2:
	add	$0x10,bl
	loop	1b

	test	si,si			// If no active, diskless boot
	jnz	1f
	int	$0x18
1:
//	mov	(si),dl
	mov	$0x55aa,bx		// Read boot sector
	push	dx
	mov	$0x41,ah
	int	$0x13
	pop	dx
	jc	1f
	cmp	$0xaa55,bx
	jne	1f
	test	$0x1,cl
	jz	1f
	mov	8(si),ax
	mov	10(si),bx
	push	si
	mov	$pkt,si
	mov	ax,8(si)
	mov	bx,10(si)
	mov	$0x42,ah
	int	$0x13
	pop	si
	jc	err_rd
	mov	$0x7c00,bx
	jmp	2f

1:
	mov	1(si),dh
	mov	2(si),cx
	mov	$0x7c00,bx
	mov	$0x201,ax
	int	$0x13
	jc	err_rd

2:
	cmpw	$0xaa55,BSIZE-2(bx)	// If magic, branch to it
	jne	err_os
	jmp	*bx

err_pt:
	mov	$msg_pt,si
	jmp	error

err_rd:
	mov	$msg_rd,si
	jmp	error

err_os:
	mov	$msg_os,si

error:
	lodsb
	test	al,al
1:
	jz	1b
	mov	$0x7,bx
	mov	$0xe,ah
	int	$0x10
	jmp	error

msg_pt: .asciz "Invalid partition table"
msg_rd: .asciz "Error loading operating system"
msg_os: .asciz "Missing operating system"

.align 8
pkt:	.byte 16
	.byte 0
	.byte 1
	.byte 0
	.word 0x7c00
	.word 0
	.long 0
	.long 0

	.align 128
	.space 190,0xcc

partab: .space	64,0			// Partition table
	.word	MAGIC			// Magic number
