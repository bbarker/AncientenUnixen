/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 2007 Robert Nordier. All rights reserved. */

#
/*
 *
 *	UNIX debugger
 *
 */

#include "defs.h"


MSG		BADNAM;
MSG		BADMAG;

MAP		txtmap;
MAP		datmap;
SYMSLAVE	*symvec;
INT		wtflag;
INT		fcor;
INT		fsym;
L_INT		maxfile;
L_INT		maxstor;
L_INT		txtsiz;
L_INT		datsiz;
L_INT		datbas;
L_INT		stksiz;
STRING		errflg;
INT		magic;
L_INT		symbas;
L_INT		symnum;
L_INT		entrypt;

INT		argcount;
INT		signo;
struct user u;
L_INT		u_stk[1024];

STRING		symfil	= "a.out";
STRING		corfil	= "core";

#define TXTHDRSIZ	(sizeof(txthdr))

setsym()
{
	INT		symval, symflg;
	SYMSLAVE	*symptr;
	SYMPTR		symp;
	TXTHDR		txthdr;

	fsym=getfile(symfil,1);
	txtmap.ufd=fsym;
	IF read(fsym, txthdr, TXTHDRSIZ)==TXTHDRSIZ
	THEN	magic=txthdr[0];
		IF magic!=0410 ANDF magic!=0407
		THEN	magic=0;
		ELSE	symnum=txthdr[4]/SYMTABSIZ;
			txtsiz=txthdr[1];
			datsiz=txthdr[2];
			symbas=txtsiz+datsiz;
			txtmap.b1=0;
			txtmap.e1=(magic==0407?symbas:txtsiz);
			txtmap.f1 = TXTHDRSIZ;
			txtmap.b2=datbas=(magic==0410?round(txtsiz,TXTRNDSIZ):0);
			txtmap.e2=txtmap.b2+(magic==0407?symbas:datsiz);
			txtmap.f2 = TXTHDRSIZ+(magic==0407?0:txtmap.e1);
			entrypt=txthdr[5];
			symbas += txthdr[6]+txthdr[7]+TXTHDRSIZ;

			/* set up symvec */
			symvec=(SYMSLAVE *)sbrk(shorten((1+symnum))*sizeof (SYMSLAVE));
			IF (symptr=symvec)==(SYMSLAVE *)-1
			THEN	printf("%s\n",BADNAM);
				symptr=symvec=(SYMSLAVE *)sbrk(sizeof (SYMSLAVE));
			ELSE	symset();
				WHILE (symp=symget()) ANDF errflg==0
				DO 
				    symflg=symp->symf;
				    symptr->valslave=symp->symv;
				    symptr->typslave=SYMTYPE(symflg);
				    symptr++;
				OD
			FI
			symptr->typslave=ESYM;
		FI
	FI
	IF magic==0 THEN txtmap.e1=maxfile; FI
}

setcor()
{
	INT 	ok;

	fcor=getfile(corfil,2);
	datmap.ufd=fcor;
	ok = 0;
	IF read(fcor, &u, sizeof(u))==sizeof(u)
	THEN
		lseek(fcor, ctob(1), 0);
		IF read(fcor, u_stk, sizeof(u_stk))==sizeof(u_stk)
		THEN ok = 1;
		FI
	FI
	IF ok
	THEN	txtsiz = ctob(u.u_tsize);
		datsiz = ctob(u.u_dsize);
		stksiz = ctob(u.u_ssize);
		datmap.b1 = datbas = (magic==0410?round(txtsiz,TXTRNDSIZ):0);
		datmap.e1=(magic==0407?txtsiz:datmap.b1)+datsiz;
		datmap.f1 = ctob(USIZE);
		datmap.b2 = maxstor-stksiz;
		datmap.e2 = maxstor;
		datmap.f2 = ctob(USIZE)+(magic==0410?datsiz:datmap.e1);
		signo = u_stk[1024-7];
		IF magic ANDF magic!=u.u_exdata.ux_mag
		THEN	printf("%s\n",BADMAG);
		FI
	ELSE	datmap.e1 = maxfile;
	FI
}

create(f)
STRING	f;
{	int fd;
	IF (fd=creat(f,0644))>=0
	THEN close(fd); return(open(f,wtflag));
	ELSE return(-1);
	FI
}

getfile(filnam,cnt)
STRING	filnam;
{
	REG INT		fsym;

	IF !eqstr("-",filnam)
	THEN	fsym=open(filnam,wtflag);
		IF fsym<0 ANDF argcount>cnt
		THEN	IF wtflag
			THEN	fsym=create(filnam);
			FI
			IF fsym<0
			THEN printf("cannot open `%s'\n", filnam);
			FI
		FI
	ELSE	fsym = -1;
	FI
	return(fsym);
}
