/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 2007 Robert Nordier. All rights reserved. */

#
/*
 *
 *	UNIX debugger - common definitions
 *
 */



/*	Layout of a.out file (fsym):
 *
 *	header of 8 words	magic number 407 or 410
 *				text size	)
 *				data size	) in bytes
 *				bss size	)
 *				symbol table size
 *				entry address
 *				size of text relocation info
 *				size of data relocation info
 *
 *
 *	header:		0
 *	text:		32
 *	data:		32+textsize
 *	text reloc:	32+textsize+datasize
 *	data reloc:	32+textsize+datasize+textreloc
 *	symbol table:	32+textsize+datasize+textreloc+datareloc
 *
 */

#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include "mac.h"
#include "mode.h"


#define VARB	11
#define VARD	13
#define VARE	14
#define VARM	22
#define VARS	28
#define VART	29

#define RD	0
#define WT	1
#define NSP	0
#define	ISP	1
#define	DSP	2
#define STAR	4
#define STARCOM 0200
#define DSYM	4
#define ISYM	4
#define NSYM	0
#define ESYM	(-1)
#define BKPTSET	1
#define BKPTEXEC 2
#define	SYMSIZ	100

#define USERPS	PSL
#define USERPC	PC
#define BPT	0314
#define TBIT	020
#define FD	0200
#define	SETTRC	0
#define	RDUSER	2
#define	RIUSER	1
#define	WDUSER	5
#define WIUSER	4
#define	RUREGS	3
#define	WUREGS	6
#define	CONTIN	7
#define	EXIT	8
#define SINGLE	9

#define FROFF	(&(0->fpsr))
#define FRLEN	25
#define FRMAX	6

/* these are all on the kernel stack */
#define EFL	(1024-3)
#define EIP	(1024-5)
#define EAX	(1024-10)
#define EBX	(1024-13)
#define ECX	(1024-11)
#define EDX	(1024-12)
#define ESI	(1024-14)
#define EDI	(1024-15)
#define EBP	(1024-19)
#define ESP	(1024-2)
#define CS	(1024-4)
#define DS	(1024-9)
#define ES	(1024-8)
#define SS	(1024-1)

#define PC	EIP
#define FP	EBP

#define MAXOFF	255
#define MAXPOS	80
#define MAXLIN	128
#define EOR	'\n'
#define QUOTE	0200
#define EVEN	-2


/* long to ints and back (puns) */
union {
	INT	I[2];
	L_INT	L;
} itolws;

#define leng(a)		itol(0,a)
#define shorten(a)	((short)(a))
#define itol(a,b)	(itolws.I[0]=(b), itolws.I[1]=(a), itolws.L)



/* result type declarations */
L_INT		inkdot();
SYMPTR		lookupsym();
SYMPTR		symget();
POS		get();
POS		chkget();
STRING		exform();
L_INT		round();
BKPTR		scanbkpt();
VOID		fault();

#include <setjmp.h>
jmp_buf erradb;
