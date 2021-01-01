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


MSG		BADFIL;

SYMTAB		symbol;
BOOL		localok;
ADDR		lastframe;
SYMSLAVE		*symvec;
ADDR		maxoff;
ADDR		maxstor;

/* symbol management */
L_INT		symbas;
L_INT		symcnt;
L_INT		symnum;
ADDR		localval;
char		symrqd = -1;
SYMTAB		symbuf[SYMSIZ];
SYMPTR		symnxt;
SYMPTR		symend;


INT		fsym;
STRING		errflg;
POS		findsym();


/* symbol table and file handling service routines */

longseek(f, a)
L_INT a;
{
	return(lseek(f,a,0) != -1);
}

valpr(v,idsp)
{
	POS		d;
	d = findsym(v,idsp);
	IF d < maxoff
	THEN	printf("%.8s", symbol.symc);
		IF d
		THEN	printf(OFFMODE, d);
		FI
	FI
}

localsym(cframe)
ADDR cframe;
{
	INT symflg;
	WHILE nextsym()
		ANDF symbol.symc[0]!='_'
		ANDF (symflg=symbol.symf)!=N_FN
	DO IF symflg>=N_TEXT ANDF symflg<=(N_BSS|N_EXT)
	   THEN localval=symbol.symv;
		return(TRUE);
	   ELIF symflg==N_ABS
	   THEN localval=cframe+symbol.symv;
		return(TRUE);
	   FI
	OD
	return(FALSE);
}
psymoff(v,type,s)
L_INT v; int type; char *s;
{
	POS		w;
	IF (v!=0) THEN w = findsym(v,type); FI
	IF v==0 ORF w >= maxoff ORF (shorten(v)<maxoff ANDF w!=0)
	THEN printf(LPRMODE,v);
	ELSE printf("%.8s", symbol.symc);
	     IF w THEN printf(OFFMODE,w); FI
	FI
	printf(s);
}

POS
findsym(svalue,type)
L_INT	svalue;
INT	type;
{
	L_INT		diff, value, symval, offset;
	INT		symtyp;
	REG SYMSLAVE	*symptr;
	SYMSLAVE	*symsav;
	value=svalue; diff = 0x7fffffffL; symsav=0;
	IF type!=NSYM ANDF (symptr=symvec)
	THEN	WHILE diff ANDF (symtyp=symptr->typslave)!=ESYM
		DO  IF (type==0 ORF symtyp==type)
		    THEN symval=symptr->valslave;
			 IF value-symval<diff
			    ANDF value>=symval
			 THEN diff = value-symval;
			      symsav=symptr;
			 FI
		    FI
		    symptr++;
		OD
		IF symsav
		THEN	offset=leng(symsav-symvec);
			symcnt=symnum-offset;
			longseek(fsym, symbas+offset*SYMTABSIZ);
			read(fsym,&symbol,SYMTABSIZ);
		FI
	FI
	return(diff);
}

nextsym()
{
	IF (--symcnt)<0
	THEN	return(FALSE);
	ELSE	return(longseek(fsym, symbas+(symnum-symcnt)*SYMTABSIZ)!=0 ANDF
			read(fsym,&symbol,SYMTABSIZ)==SYMTABSIZ);
	FI
}



/* sequential search through file */
symset()
{
	symcnt = symnum;
	symnxt = symbuf;
	IF symrqd
	THEN	longseek(fsym, symbas);
		symread(); symrqd=FALSE;
	ELSE	longseek(fsym, symbas+sizeof symbuf);
	FI
}

SYMPTR	symget()
{
	REG INT	rc;
	IF symnxt >= symend
	THEN	rc=symread(); symrqd=TRUE;
	ELSE	rc=TRUE;
	FI
	IF --symcnt>0 ANDF rc==0 THEN errflg=BADFIL; FI
	return( (symcnt>=0 && rc) ? symnxt++ : 0);
}

symread()
{
	INT		symlen;

	symlen=read(fsym,symbuf,sizeof symbuf);
	IF symlen>=SYMTABSIZ
	THEN	symnxt = symbuf;
		symend = &symbuf[symlen/SYMTABSIZ];
		return(TRUE);
	ELSE	return(FALSE);
	FI
}
