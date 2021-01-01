// V7/x86 source code: see www.nordier.com/v7x86 for details.
// Copyright (c) 2007 Robert Nordier.  All rights reserved.

// primary bootstrap

	.code16

	.set partid,0x72
	.set boot,start+1024

start:
	jmp	start1
	nop

// boot sector bios parameter block

	.ascii	"v7/x86  "
	.word	512		// bytes per sector
	.byte	0
	.word	0
	.byte	0
	.word	0
	.word	0
	.byte	0		// media descriptor
	.word	0
	.word	18		// sectors per track
	.word	2		// drive heads
hidden: .word	0		// hidden sectors
	.word	0
	.word	0
	.word	0

drv:	.byte	0		// drive number

	.align 2

start1:
	xor	ax,ax
	mov	ax,es
	mov	ax,ds
	mov	ax,ss
	mov	$start-512,sp
	cld

	mov	dl,drv
	test	dl,dl
	jns	main

	cmpb	$partid,0x4(si)
	je	2f
	call	rblk
	mov	$buf+0x1be,si
	mov	$4,cx
1:	cmpb	$partid,0x4(si)
	je	2f
	add	$0x10,si
	loop	1b
	jmp	main

2:	mov	8(si),ax
	mov	ax,hidden
	mov	10(si),ax
	mov	ax,hidden+2

main:
	mov	$2,ax
	call	iget
1:	call	rmblk
	jnc	2f
	jmp	errnb
2:	mov	$buf,dx
2:	mov	dx,si
	lodsw
	test	ax,ax
	je	3f
	mov	$prog,di
	mov	$0x5,cx
	repe
	cmpsb
	je	1f
3:	add	$0x10,dx
	cmp	$buf+512,dx
	jb	2b
	jmp	1b

1:	mov	cx,bno
	call	iget
	mov	$boot,di
1:	call	rmblk
	jb	1f
	mov	$buf,si
2:	movsw
	cmp	$buf+512,si
	jb	2b
	jmp	1b

1:	call	boot
	jmp	start

// get the inode specified in ax
// uses: ax, si, di
iget:
	add	$15,ax
	mov	ax,si
	shr	$3,ax
	mov	ax,dno
	xor	ax,ax
	call	rblk
	and	$7,si
	shl	$6,si
	add	$buf,si
	mov	$inod,di
1:	movsw
	cmp	$inod+64,di
	jb	1b
	ret

// read in a mapped block
// file offset is in bno
// Uses ax, si
rmblk:
	mov	bno,ax
	cmp	$10,ax
	jb	1f
	mov	$10,ax
1:	mov	$addr,si
	add	ax,si
	shl	ax
	add	ax,si
	lodsw
	mov	ax,dno
	lodsb
	and	$0xff,ax
	jne	1f
	cmp	ax,dno
	je	2f
1:	call	rblk
	mov	bno,ax
	incw	bno
	sub	$10,ax
	jb	2f
	mov	$buf,si
	shl	$2,ax
	add	ax,si
	lodsw
	mov	ax,dno
	lodsw
	test	ax,ax
	jne	rblk
	cmp	ax,dno
	jne	rblk
2:	cmc
	ret

// interface to pc bios floppy disk driver
// low order address in dno,
// high order in ax
rblk:
	pusha
	mov	$pkt,si
	mov	dno,dx
	add	hidden,dx
	adc	hidden+2,ax
	mov	dx,0x8(si)
	mov	ax,0xa(si)
	mov	drv,dl
	call	read
	popa
	ret

errnb:
	mov	$msgnb,si
	jmp	error
errio:
	mov	$msgio,si

error:
	mov	$'?',al
1:	mov	$0x7,bx
	mov	$0xe,ah
	int	$0x10
	lodsb
	test	al,al
	jnz	1b
1:	jmp	1b

read:
	test	dl,dl
	jns	read1
	mov	$0x55aa,bx	// try to use bios extensions
	push	dx
	mov	$0x41,ah
	int	$0x13
	pop	dx
	jc	read1
	cmp	$0xaa55,bx
	jne	read1
	test	$0x1,cl
	jz	read1
	mov	$0x42,ah
	int	$0x13
	jc	errio
	ret

read1:
	push	dx
	mov	$0x8,ah
	int	$0x13
	jc	errio
	and	$0x3f,cl
	jz	errio
	mov	dh,ch
	mov	0x8(si),ax	// lba
	mov	0xa(si),dx
	xor	bh,bh
	mov	cl,bl
	call	div
	xchg	ch,bl
	inc	bx
	call	div
	test	dx,dx
	jnz	errio
	cmp	$0x3ff,ax
	ja	errio
	xchg	ah,al
	ror	$0x2,al
	or	ch,al
	inc	ax
	xchg	ax,cx
	pop	dx
	mov	bl,dh
	mov	$5,di
1:	les	0x4(si),bx	// buf
	mov	0x2(si),al	// cnt
	mov	$0x2,ah
	int	$0x13
	jae	1f
	dec	di
	jz	errio
	xor	ah,ah
	int	$0x13
	jmp	1b
1:	ret

// divide dx:ax by bx, leaving the remainder in bx
div:
	push	cx
	xor	cx,cx
	xchg	ax,cx
	xchg	ax,dx
	div	bx
	xchg	ax,cx
	div	bx
	xchg	dx,cx
	mov	cx,bx
	pop	cx
	ret

	.align	2

pkt:	.word	16
	.word	1
	.word	buf
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0

bno:	.word	0
dno:	.word	0

prog:	.asciz	"boot"
msgnb:	.asciz	"nb"
msgio:	.asciz	"io"

	.space	7,0xcc
	.word	0xaa55
end:

	.set	buf,start-512
	.set	inod,start+512
	.set	addr,inod+12
