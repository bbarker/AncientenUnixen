/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 2007 Robert Nordier. All rights reserved. */

#
/*
 *
 *	UNIX debugger
 *
 */

#include "defs.h"


MSG		NOCFN;

ADDR		callpc;
BOOL		localok;
SYMTAB		symbol;

STRING		errflg;

findroutine(cframe)
	ADDR		cframe;
{
	ADDR		addr;
	L_INT		inst;

	localok=FALSE;
	callpc=get(cframe+4, DSP);
	/* currently we handle only call rel32 */
	IF (get(callpc - 5, ISP) & LOBYTE) != 0xe8
	THEN	errflg=NOCFN;
		return(0);
	FI
	addr = callpc + get(callpc - 4, ISP);
	IF findsym(addr,ISYM) == -1
	THEN	symbol.symc[0] = '?';
		symbol.symc[1] = 0;
		symbol.symv = 0;
	ELSE	localok=TRUE;
	FI
	inst = get(callpc, ISP) & 0xffff;
	IF inst == 0x5959		/* pop ecx; pop ecx */
	THEN	return 2;
	FI
	IF (inst&LOBYTE) == 0x59	/* pop ecx */
	THEN	return 1;
	FI
	IF inst == 0xc483		/* add $n,esp */
	THEN	return (get(callpc + 2, ISP)&LOBYTE) / 4;
	FI
	return 0;
}

