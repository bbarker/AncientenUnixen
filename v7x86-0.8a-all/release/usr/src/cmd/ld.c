/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 1999 Robert Nordier. All rights reserved. */

/*
 *  link editor
 */

#include <signal.h>
#include "sys/types.h"
#include "sys/stat.h"

#include <stdio.h>

extern char *malloc();

union bwl {
	char b;
	short w;
	int l;
};

/*	Layout of a.out file :
 *
 *	header of 8 words	magic number 407 or 410
 *				text size	)
 *				data size	) in bytes
 *				bss size	)
 *				symbol table size
 *				entry point
 *				text relocation size
 *				data relocation size
 *
 *
 *	header:			0
 *	text:			32
 *	data:			32 + text
 *	text relocation:	32 + text + data
 *	data relocation:	32 + text + data + trsize
 *	symbol table:		32 + text + data + trsize + drsize
 *
 */
#define TRUE	1
#define FALSE	0


#define	ARCMAGIC 0177545
#define OMAGIC	0405
#define	FMAGIC	0407
#define	NMAGIC	0410
#define	IMAGIC	0411

#define	EXTERN	01
#define	UNDEF	00
#define	ABS	02
#define	TEXT	04
#define	DATA	06
#define	BSS	010
#define	COMM	012	/* internal use only */

#define RPC	01
#define RLEN	06
#define	REXT	010

#define	NROUT	256
#define	NSYM	1103
#define	NSYMPR	1000

char	premeof[] = "Premature EOF";
char	goodnm[] = "__.SYMDEF";

/* table of contents stuff */
#define TABSZ	700
struct tab
{	char cname[8];
	long cloc;
} tab[TABSZ];
int tnum;

/* input management */
struct page {
	int	nuser;
	int	bno;
	int	nibuf;
	int	buff[128];
} page[2];

struct {
	int	nuser;
	int	bno;
} fpage;

struct stream {
	int	*ptr;
	int	bno;
	int	nibuf;
	int	size;
	struct page	*pno;
};

struct stream text;
struct stream reloc;

struct {
	char	aname[14];
	long	atime;
	char	auid, agid;
	int	amode;
	long	asize;
} archdr;

struct {
	int	fmagic;
	int	tsize;
	int	dsize;
	int	bsize;
	int	ssize;
	int	entry;
	int	trsize;
	int	drsize;
} filhdr;

struct relinf {
	int	addr;
	short	value;
	char	spare;
	char	flags;
};

/* one entry for each archive member referenced;
 * set in first pass
 */
struct liblist {
	long	loc;
};

struct liblist	liblist[NROUT];
struct liblist	*libp = liblist;


/* symbol management */
struct symbol {
	char	sname[8];
	char	stype;
	char	sother;
	short	sdesc;
	int	svalue;
};

struct local {
	int locindex;		/* index to symbol in file */
	struct symbol *locsymbol;	/* ptr to symbol table */
};

struct symbol	cursym;			/* current symbol */
struct symbol	symtab[NSYM];		/* actual symbols */
struct symbol	**symhash[NSYM];	/* ptr to hash table entry */
struct symbol	*lastsym;		/* last symbol entered */
int	symindex;		/* next available symbol table entry */
struct symbol	*hshtab[NSYM+2];	/* hash table for symbols */
struct local	local[NSYMPR];

/* internal symbols */
struct symbol	*p_etext;
struct symbol	*p_edata;
struct symbol	*p_end;
struct symbol	*entrypt;

int	trace;
/* flags */
int	xflag;		/* discard local symbols */
int	Xflag;		/* discard locals starting with 'L' */
int	Sflag;		/* discard all except locals and globals*/
int	rflag;		/* preserve relocation bits, don't define common */
int	arflag;		/* original copy of rflag */
int	sflag;		/* discard all symbols */
int	nflag;		/* pure procedure */
int	Oflag;		/* set magic # to 0405 (overlay) */
int	dflag;		/* define common even with rflag */
int	iflag;		/* I/D space separated */

int	ofilfnd;
char	*ofilename = "l.out";
int	infil;
char	*filname;

/* cumulative sizes set in pass 1 */
int	tsize;
int	dsize;
int	bsize;
int	ssize;
int	trsize;
int	drsize;

/* symbol relocation; both passes */
int	ctrel;
int	cdrel;
int	cbrel;

/* start address of all text; 0 unless given by -T */
int	tbase;

int	errlev;
int	delarg	= 4;
char	tfname[] = "/tmp/ldaXXXXX";


/* output management */
struct buf {
	int	fildes;
	int	nleft;
	int	*xnext;
	int	iobuf[128];
};
struct buf	toutb;
struct buf	doutb;
struct buf	troutb;
struct buf	droutb;
struct buf	soutb;

struct symbol	**lookup();
struct symbol	**slookup();
struct symbol	*lookloc();

delexit()
{
	unlink("l.out");
	if (delarg==0)
		chmod(ofilename, 0777 & ~umask(0));
	exit(delarg);
}

main(argc, argv)
char **argv;
{
	register int c, i; 
	int num;
	register char *ap, **p;
	int found; 
	int vscan; 
	char save;

	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, delexit);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, delexit);
	if (argc == 1)
		exit(4);
	p = argv+1;

	/* scan files once to find symdefs */
	for (c=1; c<argc; c++) {
		if (trace) printf("%s:\n", *p);
		filname = 0;
		ap = *p++;

		if (*ap == '-') {
			for (i=1; ap[i]; i++) {
			switch (ap[i]) {
			case 'o':
				if (++c >= argc)
					error(2, "Bad output file");
				ofilename = *p++;
				ofilfnd++;
				continue;

			case 'u':
			case 'e':
				if (++c >= argc)
					error(2, "Bad 'use' or 'entry'");
				enter(slookup(*p++));
				if (ap[i]=='e')
					entrypt = lastsym;
				continue;

			case 'D':
				if (++c >= argc)
					error(2, "-D: arg missing");
				num = atoi(*p++);
				if (dsize>num)
					error(2, "-D: too small");
				dsize = num;
				continue;

			case 'T':
				if (++c >= argc)
					error(2, "-T: arg missing");
				if (tsize!=0)
					error(2, "-T: too late");
				tbase = htoi(*p++);
				continue;

			case 'l':
				save = ap[--i]; 
				ap[i]='-';
				load1arg(&ap[i]); 
				ap[i]=save;
				break;

			case 'x':
				xflag++;
				continue;

			case 'X':
				Xflag++;
				continue;

			case 'S':
				Sflag++; 
				continue;

			case 'r':
				rflag++;
				arflag++;
				continue;

			case 's':
				sflag++;
				xflag++;
				continue;

			case 'n':
				nflag++;
				continue;

			case 'd':
				dflag++;
				continue;

			case 'i':
				iflag++;
				continue;

			case 'O':
				Oflag++;
				continue;

			case 't':
				trace++;
				continue;

			default:
				error(2, "bad flag");
			} /*endsw*/
			break;
			} /*endfor*/
		} else
			load1arg(ap);
	}
	endload(argc, argv);
}

htoi(p)
	register char *p;
{
	register int c, n;

	n = 0;
	while (c = *p++) {
		n <<= 4;
		if (c >= '0' && c <= '9')
			n += c - '0';
		else if (c >= 'A' && c <= 'F')
			n += 10 + (c - 'A');
		else if (c >= 'a' && c <= 'f')
			n += 10 + (c - 'a');
		else
			error(2, "badly formed hex number");
	}
	return (n);
}

/* used after pass 1 */
int	nsym;
int	torigin;
int	dorigin;
int	borigin;

endload(argc, argv)
int argc; 
char **argv;
{
	register int c, i; 
	int dnum;
	register char *ap, **p;
	filname = 0;
	middle();
	setupout();
	p = argv+1;
	libp = liblist;
	for (c=1; c<argc; c++) {
		ap = *p++;
		if (trace) printf("%s:\n", ap);
		if (*ap == '-') {
			for (i=1; ap[i]; i++) {
			switch (ap[i]) {
			case 'D':
				for (dnum = atoi(*p); dorigin<dnum; dorigin += 4)
					putw(0, &doutb);
			case 'T':
			case 'u':
			case 'e':
			case 'o':
				++c; 
				++p;

			default:
				continue;

			case 'l':
				ap[--i]='-'; 
				load2arg(&ap[i]);
				break;
			} /*endsw*/
			break;
			} /*endfor*/
		} else
			load2arg(ap);
	}
	finishout();
}

/* scan file to find defined symbols */
load1arg(acp)
char *acp;
{
	register char *cp;
	long nloc;

	cp = acp;
	switch ( getfile(cp)) {
	case 0:
		load1(0, 0L);
		break;

	/* regular archive */
	case 1:
		nloc = 1;
		while ( step(nloc))
			nloc += (archdr.asize + sizeof(archdr) + 1) >> 2;
		break;

	/* table of contents */
	case 2:
		tnum = archdr.asize / sizeof(struct tab);
		if (tnum >= TABSZ) {
			error(2, "fast load buffer too small");
		}
		lseek(infil, (long)(sizeof(filhdr.fmagic)+sizeof(archdr)), 0);
		read(infil, (char *)tab, tnum * sizeof(struct tab));
		while (ldrand());
		libp->loc = -1;
		libp++;
		break;
	/* out of date table of contents */
	case 3:
		error(0, "out of date (warning)");
		for(nloc = 1+((archdr.asize+sizeof(archdr)+1) >> 2); step(nloc);
			nloc += (archdr.asize + sizeof(archdr) + 1) >> 2);
		break;
	}
	close(infil);
}

step(nloc)
long nloc;
{
	dseek(&text, nloc, sizeof archdr);
	if (text.size <= 0) {
		libp->loc = -1;
		libp++;
		return(0);
	}
	mget(&text, (int *)&archdr, sizeof archdr);
	if (load1(1, nloc + (sizeof archdr) / 4)) {
		libp->loc = nloc;
		libp++;
	}
	return(1);
}

ldrand()
{
	int i;
	struct symbol *sp, **pp;
	struct liblist *oldp = libp;
	for(i = 0; i<tnum; i++) {
		if ((pp = slookup(tab[i].cname)) == 0)
			continue;
		sp = *pp;
		if (sp->stype != EXTERN+UNDEF)
			continue;
		step(tab[i].cloc >> 2);
	}
	return(oldp != libp);
}

add(a,b,s)
int a, b;
char *s;
{
	int r;

	r = (unsigned)a + (unsigned)b;
	if (r < 0)
		error(1,s);
	return(r);
}


/* single file or archive member */
load1(libflg, loc)
long loc;
{
	register struct symbol *sp;
	int savindex;
	int ndef, nloc, type, mtype;

	readhdr(loc);
	ctrel = tsize;
	cdrel += dsize;
	cbrel += bsize;
	ndef = 0;
	nloc = sizeof cursym;
	savindex = symindex;
#if 0
	if (filhdr.trsize+filhdr.drsize==0) {
		error(1, "No relocation bits");
		return(0);
	}
#endif
	loc += (sizeof filhdr + filhdr.tsize + filhdr.dsize +
		filhdr.trsize + filhdr.drsize)/4;
	dseek(&text, loc, filhdr.ssize);
	while (text.size > 0) {
		mget(&text, (int *)&cursym, sizeof cursym);
		type = cursym.stype;
		if (Sflag) {
			mtype = type&037;
			if (mtype==2 || mtype>8) {
				continue;
			}
		}
		if ((type&EXTERN)==0) {
			if (Xflag==0 || cursym.sname[0]!='L')
				nloc += sizeof cursym;
			continue;
		}
		symreloc();
		if (enter(lookup()))
			continue;
		if ((sp = lastsym)->stype != EXTERN+UNDEF)
			continue;
		if (cursym.stype == EXTERN+UNDEF) {
			if (cursym.svalue > sp->svalue)
				sp->svalue = cursym.svalue;
			continue;
		}
		if (sp->svalue != 0 && cursym.stype == EXTERN+TEXT)
			continue;
		ndef++;
		sp->stype = cursym.stype;
		sp->svalue = cursym.svalue;
	}
	if (libflg==0 || ndef) {
		tsize = add(tsize,filhdr.tsize,"text overflow");
		dsize = add(dsize,filhdr.dsize,"data overflow");
		bsize = add(bsize,filhdr.bsize,"bss overflow");
		ssize = add(ssize,nloc,"symbol table overflow");
		trsize = add(trsize,filhdr.trsize,"tr overflow");
		drsize = add(drsize,filhdr.drsize,"dr overflow");
		return(1);
	}
	/*
	 * No symbols defined by this library member.
	 * Rip out the hash table entries and reset the symbol table.
	 */
	while (symindex>savindex)
		*symhash[--symindex]=0;
	return(0);
}

middle()
{
	register struct symbol *sp, *symp;
	register t, csize;
	int nund, corigin;

	torigin=0; 
	dorigin=0; 
	borigin=0;

	p_etext = *slookup("_etext");
	p_edata = *slookup("_edata");
	p_end = *slookup("_end");
	/*
	 * If there are any undefined symbols, save the relocation bits.
	 */
	symp = &symtab[symindex];
	if (rflag==0) {
		for (sp = symtab; sp<symp; sp++)
			if (sp->stype==EXTERN+UNDEF && sp->svalue==0
				&& sp!=p_end && sp!=p_edata && sp!=p_etext) {
				rflag++;
				dflag = 0;
				break;
			}
	}
	if (rflag)
		nflag = sflag = iflag = Oflag = 0;
	/*
	 * Assign common locations.
	 */
	csize = 0;
	if (dflag || rflag==0) {
		ldrsym(p_etext, tsize, EXTERN+TEXT);
		ldrsym(p_edata, dsize, EXTERN+DATA);
		ldrsym(p_end, bsize, EXTERN+BSS);
		for (sp = symtab; sp<symp; sp++)
			if (sp->stype==EXTERN+UNDEF && (t = sp->svalue)!=0) {
				sp->svalue = csize;
				sp->stype = EXTERN+COMM;
				csize = add(csize, t, "bss overflow");
			}
	}
	/*
	 * Now set symbols to their final value
	 */
	torigin = tbase;
	dorigin = tbase + tsize;
	if (nflag)
		dorigin = (dorigin+07777) & ~07777;
	if (iflag)
		dorigin = 0;
	corigin = dorigin + dsize;
	borigin = corigin + csize;
	nund = 0;
	for (sp = symtab; sp<symp; sp++) switch (sp->stype) {
	case EXTERN+UNDEF:
		errlev |= 01;
		if (arflag==0 && sp->svalue==0) {
			if (nund==0)
				printf("Undefined:\n");
			nund++;
			printf("%.8s\n", sp->sname);
		}
		continue;

	case EXTERN+ABS:
	default:
		continue;

	case EXTERN+TEXT:
		sp->svalue += torigin;
		continue;

	case EXTERN+DATA:
		sp->svalue += dorigin;
		continue;

	case EXTERN+BSS:
		sp->svalue += borigin;
		continue;

	case EXTERN+COMM:
		sp->stype = EXTERN+BSS;
		sp->svalue += corigin;
		continue;
	}
	if (sflag || xflag)
		ssize = 0;
	bsize = add(bsize, csize, "bss overflow");
	nsym = ssize / (sizeof cursym);
}

ldrsym(asp, val, type)
struct symbol *asp;
{
	register struct symbol *sp;

	if ((sp = asp) == 0)
		return;
	if (sp->stype != EXTERN+UNDEF || sp->svalue) {
		printf("%.8s: ", sp->sname);
		error(1, "Multiply defined");
		return;
	}
	sp->stype = type;
	sp->svalue = val;
}

setupout()
{
	tcreat(&toutb, 0);
	mktemp(tfname);
	tcreat(&doutb, 1);
	if (sflag==0 || xflag==0)
		tcreat(&soutb, 1);
	if (rflag) {
		tcreat(&troutb, 1);
		tcreat(&droutb, 1);
	}
	filhdr.fmagic = (Oflag ? OMAGIC :( iflag ? IMAGIC : ( nflag ? NMAGIC : FMAGIC )));
	filhdr.tsize = tsize;
	filhdr.dsize = dsize;
	filhdr.bsize = bsize;
	filhdr.ssize = sflag? 0: (ssize + (sizeof cursym)*symindex);
	if (entrypt) {
		if (entrypt->stype!=EXTERN+TEXT)
			error(1, "Entry point not in text");
		else
			filhdr.entry = entrypt->svalue;
	} else
		filhdr.entry=0;
	filhdr.trsize = rflag ? trsize : 0;
	filhdr.drsize = rflag ? drsize : 0;
	mput(&toutb, (int *)&filhdr, sizeof filhdr);
}

tcreat(buf, tempflg)
struct buf *buf;
{
	register int ufd; 
	char *nam;
	nam = (tempflg ? tfname : ofilename);
	if ((ufd = creat(nam, 0666)) < 0)
		error(2, tempflg?"cannot create temp":"cannot create output");
	close(ufd); 
	buf->fildes = open(nam, 2);
	if (tempflg)
		unlink(tfname);
	buf->nleft = sizeof(buf->iobuf)/sizeof(int);
	buf->xnext = buf->iobuf;
}

load2arg(acp)
char *acp;
{
	register char *cp;
	register struct liblist *lp;

	cp = acp;
	if (getfile(cp) == 0) {
		while (*cp)
			cp++;
		while (cp >= acp && *--cp != '/');
		mkfsym(++cp);
		load2(0L);
	} else {	/* scan archive members referenced */
		for (lp = libp; lp->loc != -1; lp++) {
			dseek(&text, lp->loc, sizeof archdr);
			mget(&text, (int *)&archdr, sizeof archdr);
			mkfsym(archdr.aname);
			load2(lp->loc + (sizeof archdr) / 4);
		}
		libp = ++lp;
	}
	close(infil);
}

load2(loc)
long loc;
{
	register struct symbol *sp;
	register struct local *lp;
	register int symno;
	int type, mtype, offs;

	readhdr(loc);
	ctrel = torigin;
	cdrel += dorigin;
	cbrel += borigin;
	/*
	 * Reread the symbol table, recording the numbering
	 * of symbols for fixing external references.
	 */
	lp = local;
	symno = -1;
	loc += (sizeof filhdr)/4;
	offs = (filhdr.tsize + filhdr.dsize +
		filhdr.trsize + filhdr.drsize)/4;
	dseek(&text, loc + offs, filhdr.ssize);
	while (text.size > 0) {
		symno++;
		mget(&text, (int *)&cursym, sizeof cursym);
		symreloc();
		type = cursym.stype;
		if (Sflag) {
			mtype = type&037;
			if (mtype==2 || mtype>8) continue;
		}
		if ((type&EXTERN) == 0) {
			if (!sflag&&!xflag&&(!Xflag||cursym.sname[0]!='L'))
				mput(&soutb, (int *)&cursym, sizeof cursym);
			continue;
		}
		if ((sp = *lookup()) == 0)
			error(2, "internal error: symbol not found");
		if (cursym.stype == EXTERN+UNDEF) {
			if (lp >= &local[NSYMPR])
				error(2, "Local symbol overflow");
			lp->locindex = symno;
			lp->locsymbol = sp;
			lp++;
			continue;
		}
		if (cursym.stype!=sp->stype || cursym.svalue!=sp->svalue) {
			printf("%.8s: ", cursym.sname);
			error(1, "Multiply defined");
		}
	}
	dseek(&text, loc, filhdr.tsize);
	dseek(&reloc, loc + quart(filhdr.tsize + filhdr.dsize), filhdr.trsize);
	load2td(lp, ctrel, ctrel - tbase, &toutb, &troutb);
	dseek(&text, loc+quart(filhdr.tsize), filhdr.dsize);
	dseek(&reloc, loc + quart(filhdr.tsize + filhdr.dsize + filhdr.trsize), filhdr.drsize);
	load2td(lp, cdrel, cdrel - tbase, &doutb, &droutb);
	torigin += filhdr.tsize;
	dorigin += filhdr.dsize;
	borigin += filhdr.bsize;
}

load2td(lp, creloc, pos, b1, b2)
struct local *lp;
struct buf *b1, *b2;
{
	struct relinf r;
	struct symbol *sp;
	union bwl *fix;
	char *td;
	int n, v;

	n = text.size << 2;
	if (n == 0)
		return;
	td = malloc(n);
	if (td == 0)
		error(2, "no core");
	mget(&text, td, n);
	while (reloc.size > 0) {
		mget(&reloc, (int *)&r, sizeof r);
		if ((r.flags & REXT) == 0)
			switch (r.value) {
			case TEXT:
				v = ctrel;
				break;
			case DATA:
				v = cdrel;
				break;
			case BSS:
				v = cbrel;
				break;
			}
		else {
			sp = lookloc(lp, r.value);
			v = sp->svalue;
			if (sp->stype==EXTERN+UNDEF)
				r.value = nsym+(sp-symtab);
			else {
				r.value = sp->stype-(EXTERN+ABS);
				r.flags &= ~REXT;
			}
		}
		if (r.flags & RPC)
		    v -= creloc;
		fix = (union bwl *)(td + r.addr);
		switch ((r.flags & RLEN) >> 1) {
		case 0:
			fix->b += v;
			break;
		case 1:
			fix->w += v;
			break;
		case 2:
			fix->l += v;
			break;
		}
		if (rflag) {
			r.addr += pos;
			mput(b2, (int *)&r, sizeof r);
		}
	}
	mput(b1, td, n);
	free(td);
}

finishout()
{
	register n, *p;

	copy(&doutb);
	if (rflag) {
		copy(&troutb);
		copy(&droutb);
	}
	if (sflag==0) {
		if (xflag==0)
			copy(&soutb);
		for (p = (int *)symtab; p < (int *)&symtab[symindex];)
			putw(*p++, &toutb);
	}
	flush(&toutb);
	close(toutb.fildes);
	if (!ofilfnd) {
		unlink("a.out");
		link("l.out", "a.out");
		ofilename = "a.out";
	}
	delarg = errlev;
	delexit();
}

copy(buf)
struct buf *buf;
{
	register f, *p, n;

	flush(buf);
	lseek(f = buf->fildes, (long)0, 0);
	while ((n = read(f, (char *)doutb.iobuf, sizeof(doutb.iobuf))) > 1) {
		n >>= 2;
		p = (int *)doutb.iobuf;
		do
			putw(*p++, &toutb);
		while (--n);
	}
	close(f);
}

mkfsym(s)
char *s;
{

	if (sflag || xflag)
		return;
	cp8c(s, cursym.sname);
	cursym.stype = 037;
	cursym.svalue = torigin;
	mput(&soutb, (int *)&cursym, sizeof cursym);
}

mget(sp, aloc, an)
struct stream *sp;
int *aloc;
{
	register *loc, n;
	register *p;

	n = an;
	n >>= 2;
	loc = aloc;
	if ((sp->nibuf -= n) >= 0) {
		if ((sp->size -= n) > 0) {
			p = sp->ptr;
			do
				*loc++ = *p++;
			while (--n);
			sp->ptr = p;
			return;
		} else
			sp->size += n;
	}
	sp->nibuf += n;
	do {
		*loc++ = get(sp);
	} 
	while (--n);
}

mput(buf, aloc, an)
struct buf *buf; 
int *aloc;
{
	register *loc;
	register n;

	loc = aloc;
	n = an>>2;
	do {
		putw(*loc++, buf);
	} 
	while (--n);
}

dseek(asp, aloc, s)
long aloc;
struct stream *asp;
{
	register struct stream *sp;
	register struct page *p;
	/* register */ long b, o;
	int n;

	b = aloc >> 7;
	o = aloc & 0177;
	sp = asp;
	--sp->pno->nuser;
	if ((p = &page[0])->bno!=b && (p = &page[1])->bno!=b)
		if (p->nuser==0 || (p = &page[0])->nuser==0) {
			if (page[0].nuser==0 && page[1].nuser==0)
				if (page[0].bno < page[1].bno)
					p = &page[0];
			p->bno = b;
			lseek(infil, (aloc & ~0177L) << 2, 0);
			if ((n = read(infil, (char *)p->buff, 512)>>2) < 0)
				n = 0;
			p->nibuf = n;
		} else
			error(2, "No pages");
	++p->nuser;
	sp->bno = b;
	sp->pno = p;
	sp->ptr = p->buff + o;
	if (s != -1)
		sp->size = quart(s);
	if ((sp->nibuf = p->nibuf-o) <= 0)
		sp->size = 0;
}

quart(i)
{
	return((i>>2)&07777777777);
}

get(asp)
struct stream *asp;
{
	register struct stream *sp;

	sp = asp;
	if (--sp->nibuf < 0) {
		dseek(sp, (long)(sp->bno + 1) << 7, -1);
		--sp->nibuf;
	}
	if (--sp->size <= 0) {
		if (sp->size < 0)
			error(2, premeof);
		++fpage.nuser;
		--sp->pno->nuser;
		sp->pno = (struct page *)&fpage;
	}
	return(*sp->ptr++);
}

getfile(acp)
char *acp;
{
	register char *cp;
	register int c;
	struct stat x;

	cp = acp; 
	infil = -1;
	archdr.aname[0] = '\0';
	filname = cp;
	if (cp[0]=='-' && cp[1]=='l') {
		if(cp[2] == '\0')
			cp = "-la";
		filname = "/usr/lib/libxxxxxxxxxxxxxxx";
		for(c=0; cp[c+2]; c++)
			filname[c+12] = cp[c+2];
		filname[c+12] = '.';
		filname[c+13] = 'a';
		filname[c+14] = '\0';
		if ((infil = open(filname+4, 0)) >= 0) {
			filname += 4;
		}
	}
	if (infil == -1 && (infil = open(filname, 0)) < 0)
		error(2, "cannot open");
	page[0].bno = page[1].bno = -1;
	page[0].nuser = page[1].nuser = 0;
	text.pno = reloc.pno = (struct page *)&fpage;
	fpage.nuser = 2;
	dseek(&text, 0L, 4);
	if (text.size <= 0)
		error(2, premeof);
	if(get(&text) != ARCMAGIC)
		return(0);	/* regular file */
	dseek(&text, 1L, sizeof archdr);	/* word addressing */
	if(text.size <= 0)
		return(1);	/* regular archive */
	mget(&text, (int *)&archdr, sizeof archdr);
	if(strncmp(archdr.aname, goodnm, 14) != 0)
		return(1);	/* regular archive */
	else {
		fstat(infil, &x);
		if(x.st_mtime > archdr.atime)
		{
			return(3);
		}
		else return(2);
	}
}

struct symbol **lookup()
{
	int i; 
	int clash;
	register struct symbol **hp;
	register char *cp, *cp1;

	i = 0;
	for (cp = cursym.sname; cp < &cursym.sname[8];)
		i = (i<<1) + *cp++;
	for (hp = &hshtab[(i&077777)%NSYM+2]; *hp!=0;) {
		cp1 = (*hp)->sname; 
		clash=FALSE;
		for (cp = cursym.sname; cp < &cursym.sname[8];)
			if (*cp++ != *cp1++) {
				clash=TRUE; 
				break;
			}
		if (clash) {
			if (++hp >= &hshtab[NSYM+2])
				hp = hshtab;
		} else
			break;
	}
	return(hp);
}

struct symbol **slookup(s)
char *s;
{
	cp8c(s, cursym.sname);
	cursym.stype = EXTERN+UNDEF;
	cursym.svalue = 0;
	return(lookup());
}

enter(hp)
struct symbol **hp;
{
	register struct symbol *sp;

	if (*hp==0) {
		if (symindex>=NSYM)
			error(2, "Symbol table overflow");
		symhash[symindex] = hp;
		*hp = lastsym = sp = &symtab[symindex++];
		cp8c(cursym.sname, sp->sname);
		sp->stype = cursym.stype;
		sp->svalue = cursym.svalue;
		return(1);
	} else {
		lastsym = *hp;
		return(0);
	}
}

symreloc()
{
	switch (cursym.stype) {

	case TEXT:
	case EXTERN+TEXT:
		cursym.svalue += ctrel;
		return;

	case DATA:
	case EXTERN+DATA:
		cursym.svalue += cdrel;
		return;

	case BSS:
	case EXTERN+BSS:
		cursym.svalue += cbrel;
		return;

	case EXTERN+UNDEF:
		return;
	}
	if (cursym.stype&EXTERN)
		cursym.stype = EXTERN+ABS;
}

error(n, s)
char *s;
{
	if (errlev==0)
		printf("ld:");
	if (filname) {
		printf("%s", filname);
		if (archdr.aname[0])
			printf("(%.14s)", archdr.aname);
		printf(": ");
	}
	printf("%s\n", s);
	if (n > 1)
		delexit();
	errlev = n;
}

struct symbol *
lookloc(alp, r)
struct local *alp;
{
	register struct local *clp, *lp;
	register sn;

	lp = alp;
	sn = r;
	for (clp = local; clp<lp; clp++)
		if (clp->locindex == sn)
			return(clp->locsymbol);
	error(2, "Local symbol botch");
}

readhdr(loc)
long loc;
{
	register st, sd;

	dseek(&text, loc, sizeof filhdr);
	mget(&text, (int *)&filhdr, sizeof filhdr);
	if (filhdr.fmagic != FMAGIC)
		error(2, "Bad format");
	st = (filhdr.tsize+010) & ~010;
	filhdr.tsize = st;
	cdrel = -st;
	sd = (filhdr.dsize+010) & ~010;
	cbrel = - (st+sd);
	filhdr.bsize = (filhdr.bsize+010) & ~010;
}

cp8c(from, to)
char *from, *to;
{
	register char *f, *t, *te;

	f = from;
	t = to;
	te = t+8;
	while ((*t++ = *f++) && t<te);
	while (t<te)
		*t++ = 0;
}

putw(w, b)
register struct buf *b;
{
	*(b->xnext)++ = w;
	if (--b->nleft <= 0)
		flush(b);
}

flush(b)
register struct buf *b;
{
	register n;

	if ((n = (char *)b->xnext - (char *)b->iobuf) > 0)
		if (write(b->fildes, (char *)b->iobuf, n) != n)
			error(2, "output error");
	b->xnext = b->iobuf;
	b->nleft = sizeof(b->iobuf)/sizeof(int);
}
