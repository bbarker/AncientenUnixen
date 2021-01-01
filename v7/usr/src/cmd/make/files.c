/* UNIX DEPENDENT PROCEDURES */


/* DEFAULT RULES FOR UNIX */

char *builtin[] =
	{
	".SUFFIXES : .out .o .c .f .e .r .y .yr .ye .l .s",
	"YACC=yacc",
	"YACCR=yacc -r",
	"YACCE=yacc -e",
	"YFLAGS=",
	"LEX=lex",
	"LFLAGS=",
	"CC=cc",
#ifdef vax
	"AS=as".
#else
	"AS=as -",
#endif
	"CFLAGS=",
	"RC=f77",
	"RFLAGS=",
	"EC=f77",
	"EFLAGS=",
	"FFLAGS=",
	"LOADLIBES=",

	".c.o :",
	"\t$(CC) $(CFLAGS) -c $<",

	".e.o .r.o .f.o :",
	"\t$(EC) $(RFLAGS) $(EFLAGS) $(FFLAGS) -c $<",

	".s.o :",
	"\t$(AS) -o $@ $<",

	".y.o :",
	"\t$(YACC) $(YFLAGS) $<",
	"\t$(CC) $(CFLAGS) -c y.tab.c",
	"\trm y.tab.c",
	"\tmv y.tab.o $@",

	".yr.o:",
	"\t$(YACCR) $(YFLAGS) $<",
	"\t$(RC) $(RFLAGS) -c y.tab.r",
	"\trm y.tab.r",
	"\tmv y.tab.o $@",

	".ye.o :",
	"\t$(YACCE) $(YFLAGS) $<",
	"\t$(EC) $(RFLAGS) -c y.tab.e",
	"\trm y.tab.e",
	"\tmv y.tab.o $@",

	".l.o :",
	"\t$(LEX) $(LFLAGS) $<",
	"\t$(CC) $(CFLAGS) -c lex.yy.c",
	"\trm lex.yy.c",
	"\tmv lex.yy.o $@",

	".y.c :",
	"\t$(YACC) $(YFLAGS) $<",
	"\tmv y.tab.c $@",

	".l.c :",
	"\t$(LEX) $<",
	"\tmv lex.yy.c $@",

	".yr.r:",
	"\t$(YACCR) $(YFLAGS) $<",
	"\tmv y.tab.r $@",

	".ye.e :",
	"\t$(YACCE) $(YFLAGS) $<",
	"\tmv y.tab.e $@",

	".s.out .c.out .o.out :",
	"\t$(CC) $(CFLAGS) $< $(LOADLIBES) -o $@",

	".f.out .r.out .e.out :",
	"\t$(EC) $(EFLAGS) $(RFLAGS) $(FFLAGS) $< $(LOADLIBES) -o $@",
	"\t-rm $*.o",

	".y.out :",
	"\t$(YACC) $(YFLAGS) $<",
	"\t$(CC) $(CFLAGS) y.tab.c $(LOADLIBES) -ly -o $@",
	"\trm y.tab.c",

	".l.out :",
	"\t$(LEX) $<",
	"\t$(CC) $(CFLAGS) lex.yy.c $(LOADLIBES) -ll -o $@",
	"\trm lex.yy.c",

	0 };

#include "defs"
#include <sys/types.h>


TIMETYPE exists(filename)
char *filename;
{
#include <sys/stat.h>
struct stat buf;
register char *s;
TIMETYPE lookarch();

for(s = filename ; *s!='\0' && *s!='(' ; ++s)
	;

if(*s == '(')
	return(lookarch(filename));

if(stat(filename,&buf) < 0) 
	return(0);
else	return(buf.st_mtime);
}


TIMETYPE prestime()
{
TIMETYPE t;
time(&t);
return(t);
}



#include <sys/dir.h>
FSTATIC char n15[15];
FSTATIC char *n15end	= &n15[14];



struct depblock *srchdir(pat, mkchain, nextdbl)
register char *pat; /* pattern to be matched in directory */
int mkchain;  /* nonzero if results to be remembered */
struct depblock *nextdbl;  /* final value for chain */
{
FILE * dirf;
int i, nread;
char *dirname, *dirpref, *endir, *filepat, *p, temp[100];
char fullname[100], *p1, *p2;
struct nameblock *q;
struct depblock *thisdbl;
struct opendir *od;
struct pattern *patp;

struct direct entry[32];


thisdbl = 0;

if(mkchain == NO)
	for(patp=firstpat ; patp ; patp = patp->nxtpattern)
		if(! unequal(pat, patp->patval)) return(0);

patp = ALLOC(pattern);
patp->nxtpattern = firstpat;
firstpat = patp;
patp->patval = copys(pat);

endir = 0;

for(p=pat; *p!='\0'; ++p)
	if(*p=='/') endir = p;

if(endir==0)
	{
	dirname = ".";
	dirpref = "";
	filepat = pat;
	}
else	{
	dirname = pat;
	*endir = '\0';
	dirpref = concat(dirname, "/", temp);
	filepat = endir+1;
	}

dirf = NULL;

for(od = firstod ; od; od = od->nxtopendir)
	if(! unequal(dirname, od->dirn) )
		{
		dirf = od->dirfc;
		fseek(dirf,0L,0); /* start over at the beginning  */
		break;
		}

if(dirf == NULL)
	{
	dirf = fopen(dirname, "r");
	od = ALLOC(opendir);
	od->nxtopendir = firstod;
	firstod = od;
	od->dirfc = dirf;
	od->dirn = copys(dirname);
	}

if(dirf == NULL)
	{
	fprintf(stderr, "Directory %s: ", dirname);
	fatal("Cannot open");
	}

else do
	{
	nread = fread( (char *) &entry[0], sizeof(struct direct), 32, dirf) ;
	for(i=0; i<nread; ++i)
		if(entry[i].d_ino!= 0)
			{
			p1 = entry[i].d_name;
			p2 = n15;
			while( (p2<n15end) &&
			  (*p2++ = *p1++)!='\0' );
			if( amatch(n15,filepat) )
				{
				concat(dirpref,n15,fullname);
				if( (q=srchname(fullname)) ==0)
					q = makename(copys(fullname));
				if(mkchain)
					{
					thisdbl = ALLOC(depblock);
					thisdbl->nxtdepblock = nextdbl;
					thisdbl->depname = q;
					nextdbl = thisdbl;
					}
				}
			}

	} while(nread==32);

if(endir != 0)  *endir = '/';

return(thisdbl);
}

/* stolen from glob through find */

static amatch(s, p)
char *s, *p;
{
	register int cc, scc, k;
	int c, lc;

	scc = *s;
	lc = 077777;
	switch (c = *p) {

	case '[':
		k = 0;
		while (cc = *++p) {
			switch (cc) {

			case ']':
				if (k)
					return(amatch(++s, ++p));
				else
					return(0);

			case '-':
				k |= (lc <= scc)  & (scc <= (cc=p[1]) ) ;
			}
			if (scc==(lc=cc)) k++;
		}
		return(0);

	case '?':
	caseq:
		if(scc) return(amatch(++s, ++p));
		return(0);
	case '*':
		return(umatch(s, ++p));
	case 0:
		return(!scc);
	}
	if (c==scc) goto caseq;
	return(0);
}

static umatch(s, p)
char *s, *p;
{
	if(*p==0) return(1);
	while(*s)
		if (amatch(s++,p)) return(1);
	return(0);
}

#ifdef METERFILE
#include <pwd.h>
int meteron	= 0;	/* default: metering off */

meter(file)
char *file;
{
TIMETYPE tvec;
char *p, *ctime();
FILE * mout;
struct passwd *pwd, *getpwuid();

if(file==0 || meteron==0) return;

pwd = getpwuid(getuid());

time(&tvec);

if( (mout=fopen(file,"a")) != NULL )
	{
	p = ctime(&tvec);
	p[16] = '\0';
	fprintf(mout,"User %s, %s\n",pwd->pw_name,p+4);
	fclose(mout);
	}
}
#endif


/* look inside archives for notations a(b) and a((b))
	a(b)	is file member   b   in archive a
	a((b))	is entry point  _b  in object archive a
*/
#include <ar.h>
#include <a.out.h>

static struct ar_hdr arhead;
FILE *arfd;
long int arpos, arlen;

static struct exec objhead;

static struct nlist objentry;


TIMETYPE lookarch(filename)
char *filename;
{
char *p, *q, *send, s[15];
int i, nc, nsym, objarch;

for(p = filename; *p!= '(' ; ++p)
	;
*p = '\0';
openarch(filename);
*p++ = '(';

if(*p == '(')
	{
	objarch = YES;
	nc = 8;
	++p;
	}
else
	{
	objarch = NO;
	nc = 14;
	}
send = s + nc;

for( q = s ; q<send && *p!='\0' && *p!=')' ; *q++ = *p++ )
	;
while(q < send)
	*q++ = '\0';
while(getarch())
	{
	if(objarch)
		{
		getobj();
		nsym = objhead.a_syms / sizeof(objentry);
		for(i = 0; i<nsym ; ++i)
			{
			fread( (char *) &objentry, sizeof(objentry),1,arfd);
			if( (objentry.n_type & N_EXT)
			   && ((objentry.n_type & ~N_EXT) || objentry.n_value)
			   && eqstr(objentry.n_name,s,nc))
				{
				clarch();
				return(arhead.ar_date);
				}
			}
		}

	else if( eqstr(arhead.ar_name, s, nc))
		{
		clarch();
		return( arhead.ar_date);
		}
	}

clarch();
return( 0L);
}


clarch()
{
fclose( arfd );
}


openarch(f)
register char *f;
{
int word;
#include <sys/stat.h>
struct stat buf;

stat(f, &buf);
arlen = buf.st_size;

arfd = fopen(f, "r");
if(arfd == NULL)
	fatal1("cannot open %s", f);
fread( (char *) &word, sizeof(word), 1, arfd);
if(word != ARMAG)
	fatal1("%s is not an archive", f);
arpos = 0;
arhead.ar_size = 2 - sizeof(arhead);
}



getarch()
{
arpos += sizeof(arhead);
arpos += (arhead.ar_size + 1 ) & ~1L;
if(arpos >= arlen)
	return(0);
fseek(arfd, arpos, 0);
fread( (char *) &arhead, sizeof(arhead), 1, arfd);
return(1);
}


getobj()
{
long int skip;

fread( (char *) &objhead, sizeof(objhead), 1, arfd);
if( objhead.a_magic != A_MAGIC1 &&
    objhead.a_magic != A_MAGIC2 &&
    objhead.a_magic != A_MAGIC3 &&
    objhead.a_magic != A_MAGIC4 )
		fatal1("%s is not an object module", arhead.ar_name);
skip = objhead.a_text + objhead.a_data;
if(! objhead.a_flag )
	skip *= 2;
fseek(arfd, skip, 1);
}


eqstr(a,b,n)
register char *a, *b;
int n;
{
register int i;
for(i = 0 ; i < n ; ++i)
	if(*a++ != *b++)
		return(NO);
return(YES);
}
