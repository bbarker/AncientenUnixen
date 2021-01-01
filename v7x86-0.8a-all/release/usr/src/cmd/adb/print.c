/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 2007 Robert Nordier. All rights reserved. */

#
/*
 *
 *	UNIX debugger
 *
 */

#include "defs.h"
#include "a.out.h"
struct user u;
L_INT		u_stk[1024];


MSG		LONGFIL;
MSG		NOTOPEN;
MSG		BADMOD;

MAP		txtmap;
MAP		datmap;

SYMTAB		symbol;
ADDR		lastframe;
ADDR		callpc;

INT		infile;
INT		outfile;
CHAR		*lp;
L_INT		maxoff;
L_INT		maxpos;
INT		radix;

/* symbol management */
L_INT		localval;

/* breakpoints */
BKPTR		bkpthead;

REGLIST reglist [] = {
		"efl", EFL,
		"eip", EIP,
		"eax", EAX,
		"ebx", EBX,
		"ecx", ECX,
		"edx", EDX,
		"esi", ESI,
		"edi", EDI,
		"ebp", EBP,
		"esp", ESP,
		"cs", CS,
		"ds", DS,
		"es", ES,
		"ss", SS
};

char		lastc;

INT		fcor;
STRING		errflg;
INT		signo;


L_INT		dot;
L_INT		var[];
STRING		symfil;
STRING		corfil;
INT		pid;
L_INT		adrval;
INT		adrflg;
L_INT		cntval;
INT		cntflg;

STRING		signals[] = {
		"",
		"hangup",
		"interrupt",
		"quit",
		"illegal instruction",
		"trace/BPT",
		"IOT",
		"EMT",
		"floating exception",
		"killed",
		"bus error",
		"memory fault",
		"bad system call",
		"broken pipe",
		"alarm call",
		"terminated",
};
#define MAXSIG 15




/* general printing routines ($) */

printtrace(modif)
{
	INT		narg, i, stat, name, limit;
	POS		dynam;
	REG BKPTR	bkptr;
	CHAR		hi, lo;
	ADDR		word;
	STRING		comptr;
	ADDR		argp, frame, link;
	SYMPTR		symp;

	IF cntflg==0 THEN cntval = -1; FI

	switch (modif) {

	    case '<':
	    case '>':
		{CHAR		file[64];
		INT		index;

		index=0;
		IF modif=='<'
		THEN	iclose();
		ELSE	oclose();
		FI
		IF rdc()!=EOR
		THEN	REP file[index++]=lastc;
			    IF index>=63 THEN error(LONGFIL); FI
			PER readchar()!=EOR DONE
			file[index]=0;
			IF modif=='<'
			THEN	infile=open(file,0);
				IF infile<0
				THEN	infile=0; error(NOTOPEN);
				FI
			ELSE	outfile=open(file,1);
				IF outfile<0
				THEN	outfile=creat(file,0644);
				ELSE	lseek(outfile,0L,2);
				FI
			FI

		FI
		lp--;
		}
		break;

	    case 'd':
		if (adrflg) {
			if (adrval<2 || adrval>16) {printf("must have 2 <= radix <= 16"); break;}
			printf("radix=%d base ten",radix=adrval);
		}
		break;

	    case 'q': case 'Q': case '%':
		done();

	    case 'w': case 'W':
		maxpos=(adrflg?adrval:MAXPOS);
		break;

	    case 's': case 'S':
		maxoff=(adrflg?adrval:MAXOFF);
		break;

	    case 'v': case 'V':
		prints("variables\n");
		FOR i=0;i<=35;i++
		DO IF var[i]
		   THEN printc((i<=9 ? '0' : 'a'-10) + i);
			printf(" = %Q\n",var[i]);
		   FI
		OD
		break;

	    case 'm': case 'M':
		printmap("? map",&txtmap);
		printmap("/ map",&datmap);
		break;

	    case 0: case '?':
		IF pid
		THEN printf("pcs id = %d\n",pid);
		ELSE prints("no process\n");
		FI
		sigprint(); flushbuf();

	    case 'r': case 'R':
		printregs();
		return;

	    case 'c': case 'C':
		frame=adrflg?adrval:u_stk[FP]; lastframe=0;
		callpc=adrflg?get(frame+4,DSP):u_stk[PC];
		WHILE cntval--
		DO	chkerr();
			narg = findroutine(frame);
			printf("%.8s(", symbol.symc);
			argp = frame+8;
			IF --narg >= 0
			THEN	printf("%R", get(argp, DSP));
			FI
			WHILE --narg >= 0
			DO	argp += 4;
				printf(",%R", get(argp, DSP));
			OD
			prints(")\n");

			IF modif=='C'
			THEN WHILE localsym(frame)
			     DO word=get(localval,DSP);
				printf("%8t%.8s:%10t", symbol.symc);
				IF errflg THEN prints("?\n"); errflg=0; ELSE printf("%R\n",word); FI
			     OD
			FI

			lastframe=frame;
			frame=get(frame, DSP);
			IF frame==0 THEN break; FI
		OD
		break;

	    /*print externals*/
	    case 'e': case 'E':
		symset();
		WHILE symp=symget()
		DO chkerr();
		   IF symp->symf==(N_DATA|N_EXT) ORF symp->symf==(N_BSS|N_EXT)
		   THEN printf("%.8s:%12t%R\n", symp->symc, get(symp->symv,DSP));
		   FI
		OD
		break;

	    /*print breakpoints*/
	    case 'b': case 'B':
		printf("breakpoints\ncount%8tbkpt%24tcommand\n");
		FOR bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt
		DO IF bkptr->flag
		   THEN printf("%-8.8d",bkptr->count);
			psymoff(leng(bkptr->loc),ISYM,"%24t");
			comptr=bkptr->comm;
			WHILE *comptr DO printc(*comptr++); OD
		   FI
		OD
		break;

	    default: error(BADMOD);
	}

}

printmap(s,amap)
STRING	s; MAP *amap;
{
	int file;
	file=amap->ufd;
	printf("%s%12t`%s'\n",s,(file<0 ? "-" : (file==fcor ? corfil : symfil)));
	printf("b1 = %-16R",amap->b1);
	printf("e1 = %-16R",amap->e1);
	printf("f1 = %-16R",amap->f1);
	printf("\nb2 = %-16R",amap->b2);
	printf("e2 = %-16R",amap->e2);
	printf("f2 = %-16R",amap->f2);
	printc(EOR);
}

printregs()
{
	REG REGPTR	p;
	L_INT		v;

	FOR p=reglist; p < &reglist[14]; p++
	DO	printf("%s%6t%R %16t", p->rname, v=u_stk[p->roffs]);
		valpr(v,(p->roffs==EIP?ISYM:DSYM));
		printc(EOR);
	OD
	printpc();
}

getreg(regnam)
{
	REG REGPTR	p;
	REG STRING	regptr;
	CHAR	*olp;
	CHAR		regnxt;

	olp=lp;
	FOR p=reglist; p < &reglist[24]; p++
	DO	regptr=p->rname;
		IF (regnam == *regptr++)
		THEN
			WHILE *regptr
			DO IF (regnxt=readchar()) != *regptr++
				THEN --regptr; break;
				FI
			OD
			IF *regptr
			THEN lp=olp;
			ELSE return(p->roffs);
			FI
		FI
	OD
	lp=olp;
	return(0);
}

printpc()
{
	dot=u_stk[PC];
	psymoff(dot,ISYM,":%16t"); printins(0,ISP,chkget(dot,ISP));
	printc(EOR);
}

sigprint()
{
	IF (signo>=0) ANDF (signo<=MAXSIG) THEN prints(signals[signo]); FI
}

